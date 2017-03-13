/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/environment.h"

#include <assert.h>
#include <functional>
#include <memory>
#include <ostream>
#include <unordered_map>

#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"
#include "hst/semantic-models.h"

namespace hst {

namespace {

class Equivalences {
  public:
    using Head = const NormalizedProcess*;
    using ClassMap = std::unordered_map<const NormalizedProcess*, Head>;
    using MemberSet = std::unordered_set<const NormalizedProcess*>;
    using MemberMap = std::unordered_map<Head, MemberSet>;

    void add(Head head, const NormalizedProcess* process)
    {
        classes_[process] = head;
        members_[head].insert(process);
    }

    Head get_class(const NormalizedProcess* process) const
    {
        auto it = classes_.find(process);
        if (it == classes_.end()) {
            return nullptr;
        } else {
            return it->second;
        }
    }

    const MemberMap& get_classes() const { return members_; }
    const MemberSet& get_members(Head head) const
    {
        auto it = members_.find(head);
        assert(it != members_.end());
        return it->second;
    }

  private:
    ClassMap classes_;
    MemberMap members_;
};

template <typename Model>
Equivalences
initialize_bisimulation(const NormalizedProcess* root)
{
    Equivalences result;
    std::unordered_map<typename Model::Behavior, Equivalences::Head> behaviors;
    root->bfs([&behaviors, &result](const NormalizedProcess& process) {
        auto behavior = Model::get_process_behavior(process);
        Equivalences::Head& head = behaviors[behavior];
        if (!head) {
            // This is the first process we've encountered with this behavior,
            // so use it as the head of the equivalence class.
            head = &process;
        }
        result.add(head, &process);
        return true;
    });
    return result;
}

// Check whether two processes are shallowly equivalent: that is, whether they
// belong to the same equivalence class.
bool
processes_shallow_equiv(const Equivalences& equivalences,
                        const NormalizedProcess* p1,
                        const NormalizedProcess* p2)
{
    Equivalences::Head head1 = equivalences.get_class(p1);
    Equivalences::Head head2 = equivalences.get_class(p2);
    assert(head1 && head2);
    return head1 == head2;
}

// Check whether two processes are deeply equivalent: that all of the targets
// from both lead to processes that are shallowly equivalent.
bool
processes_deep_equiv(const Equivalences& equivalences,
                     const NormalizedProcess* p1, const NormalizedProcess* p2)
{
    Event::Set initials;
    p1->initials(&initials);
    for (Event initial : initials) {
        const NormalizedProcess* after1 = p1->after(initial);
        const NormalizedProcess* after2 = p2->after(initial);
        assert(after1 && after2);
        if (!processes_shallow_equiv(equivalences, after1, after2)) {
            return false;
        }
    }
    return true;
}

template <typename Model>
std::unique_ptr<Equivalences>
bisimulate(const NormalizedProcess* root)
{
    bool changed;
    Equivalences prev_equiv = initialize_bisimulation<Model>(root);
    Equivalences next_equiv;

    do {
        // We don't want to start another iteration after this one unless we
        // find any changes.
        changed = false;

        // Loop through each pair of states that were equivalent before,
        // verifying that they're still equivalent.  Separate any that are not
        // equivalent to their head into a new class.
        for (const auto& head_and_members : prev_equiv.get_classes()) {
            Equivalences::Head head = head_and_members.first;
            Equivalences::Head new_head = nullptr;
            const auto& members = head_and_members.second;
            assert(!members.empty());

            for (const NormalizedProcess* member : members) {
                // If we find a non-equivalent member of this class, we'll need
                // to separate it out into a new class.  This new class will
                // need a head, which will be the first non-equivalent member we
                // find.
                //
                // If we find multiple members that aren't equivalent to the
                // head, we'll put them into the same new equivalence class; if
                // they turn out to also not be equivalent to each other, we'll
                // catch that in a later iteration.

                if (processes_deep_equiv(prev_equiv, head, member)) {
                    // This process is equivalent to its previous head; keep it
                    // in the same equivalence class as head in the next round.
                    next_equiv.add(head, member);
                } else {
                    // This state is not equivalent to its previous head.  If
                    // necessary, create a new equivalence class.  Add the node
                    // to this new class.
                    if (!new_head) {
                        new_head = member;
                    }
                    next_equiv.add(new_head, member);
                    changed = true;
                }
            }
        }

        std::swap(prev_equiv, next_equiv);
    } while (changed);

    return std::unique_ptr<Equivalences>(
            new Equivalences(std::move(next_equiv)));
}

template <typename Model>
class Normalization : public NormalizedProcess {
  public:
    Normalization(Environment* env, const NormalizedProcess* prenormalized_root,
                  std::unique_ptr<Equivalences> equivalences,
                  Equivalences::Head equivalence_class)
        : env_(env),
          prenormalized_root_(prenormalized_root),
          equivalences_(equivalences.get()),
          equivalence_class_(equivalence_class),
          equivalences_owned_(std::move(equivalences))
    {
        assert(equivalence_class);
    }

    void initials(std::function<void(Event)> op) const override;
    const NormalizedProcess* after(Event initial) const override;
    void subprocesses(std::function<void(const Process&)> op) const override;
    void expand(std::function<void(const Process&)> op) const override;

    std::size_t hash() const override;
    bool operator==(const Process& other) const override;
    unsigned int precedence() const override { return 0; }
    void print(std::ostream& out) const override;

    // Only used in test cases!
    const NormalizedProcess* find_subprocess(Process::Set processes) const;

  private:
    Normalization(Environment* env, const NormalizedProcess* prenormalized_root,
                  Equivalences* equivalences,
                  Equivalences::Head equivalence_class)
        : env_(env),
          prenormalized_root_(prenormalized_root),
          equivalences_(equivalences),
          equivalence_class_(equivalence_class)
    {
        assert(equivalence_class);
    }

    const Equivalences::MemberSet& members() const
    {
        return equivalences_->get_members(equivalence_class_);
    }

    Environment* env_;
    const NormalizedProcess* prenormalized_root_;
    Equivalences* equivalences_;
    Equivalences::Head equivalence_class_;
    std::unique_ptr<Equivalences> equivalences_owned_;
};

}  // namespace

template <typename Model>
const NormalizedProcess*
Environment::normalize(const NormalizedProcess* root)
{
    std::unique_ptr<Equivalences> equivalences = bisimulate<Model>(root);
    Equivalences::Head equivalence_class = equivalences->get_class(root);
    assert(equivalence_class);
    return register_process(new Normalization<Model>(
            this, root, std::move(equivalences), equivalence_class));
}

template <typename Model>
const NormalizedProcess*
Normalization<Model>::find_subprocess(Process::Set processes) const
{
    // Find the equivalence class that `processes` belong to.
    for (const auto& head_and_members : equivalences_->get_classes()) {
        Equivalences::Head head = head_and_members.first;
        const auto& members = head_and_members.second;
        Process::Set expanded_members;
        for (const NormalizedProcess* member : members) {
            member->expand([&expanded_members](const Process& process) {
                expanded_members.insert(&process);
            });
        }
        if (processes == expanded_members) {
            // We've found the right equivalence class!
            return env_->register_process(new Normalization<Model>(
                    env_, prenormalized_root_, equivalences_, head));
        }
    }
    assert(false);
    return nullptr;

}

template <typename Model>
const NormalizedProcess*
Environment::normalize(const NormalizedProcess* root, Process::Set processes)
{
    auto* normalized_root =
            dynamic_cast<const Normalization<Model>*>(normalize<Model>(root));
    return normalized_root->find_subprocess(std::move(processes));
}

template const NormalizedProcess*
Environment::normalize<Traces>(const NormalizedProcess* root);

template const NormalizedProcess*
Environment::normalize<Traces>(const NormalizedProcess* root,
                               Process::Set processes);

template <typename Model>
void
Normalization<Model>::initials(std::function<void(Event)> op) const
{
    for (const NormalizedProcess* process : members()) {
        process->initials(op);
    }
}

template <typename Model>
const NormalizedProcess*
Normalization<Model>::after(Event initial) const
{
    // Find the set of processes that you could end up in by starting in one of
    // our underlying processes and following a single `initial` event.
    std::unordered_set<const NormalizedProcess*> afters;
    for (const NormalizedProcess* process : members()) {
        const NormalizedProcess* after = process->after(initial);
        if (after) {
            afters.insert(after);
        }
    }

    // If none of the processes can perform this event, neither can we.
    if (afters.empty()) {
        return nullptr;
    }

    // Because we've already prenormalized the underlying processes and merged
    // equivalent processes via bisimulation, all of the `afters` that we just
    // found should all belong to the same equivalence class.
    Equivalences::Head after_head = nullptr;
    for (const NormalizedProcess* after : afters) {
        Equivalences::Head curr_head = equivalences_->get_class(after);
        assert(!after_head || after_head == curr_head);
        if (!after_head) {
            after_head = curr_head;
        }
    }

    // Our "real" after is the normalized node for this equivalence class that
    // we just found.
    return env_->register_process(new Normalization<Model>(
            env_, prenormalized_root_, equivalences_, after_head));
}

template <typename Model>
void
Normalization<Model>::subprocesses(std::function<void(const Process&)> op) const
{
    for (const NormalizedProcess* process : members()) {
        op(*process);
    }
}

template <typename Model>
void
Normalization<Model>::expand(std::function<void(const Process&)> op) const
{
    for (const NormalizedProcess* process : members()) {
        process->expand(op);
    }
}

template <typename Model>
std::size_t
Normalization<Model>::hash() const
{
    static hash_scope normalized;
    return hasher(normalized)
            .add(prenormalized_root_)
            .add(equivalence_class_)
            .value();
}

template <typename Model>
bool
Normalization<Model>::operator==(const Process& other_) const
{
    const Normalization* other =
            dynamic_cast<const Normalization*>(&other_);
    if (other == nullptr) {
        return false;
    }
    return prenormalized_root_ == other->prenormalized_root_ &&
           equivalence_class_ == other->equivalence_class_;
}

template <typename Model>
void
Normalization<Model>::print(std::ostream& out) const
{
    Process::Set expansion;
    Process::Set root_expansion;
    expand([&expansion](const Process& process) {
        expansion.insert(&process);
    });
    prenormalized_root_->expand([&root_expansion](const Process& process) {
        root_expansion.insert(&process);
    });
    if (expansion == root_expansion) {
        out << "normalize[" << Model::abbreviation() << "] " << root_expansion;
    } else {
        out << "normalize[" << Model::abbreviation() << "] " << expansion
            << " within " << root_expansion;
    }
}

}  // namespace hst

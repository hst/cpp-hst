/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst/refinement.h"

#include <unordered_set>

#include "hst/environment.h"
#include "hst/event.h"
#include "hst/hash.h"
#include "hst/process.h"
#include "hst/semantic-models.h"

namespace hst {

namespace {

template <typename Model>
class RefinementPair {
  public:
    using Set = std::unordered_set<RefinementPair<Model>>;

    RefinementPair(const NormalizedProcess* spec, const Process* impl)
        : spec_(spec), impl_(impl)
    {
    }

    // Returns whether Spec's behavior refines Impl's behavior.  (This is not a
    // deep refinement check; it's used to construct the deep refinement check.)
    bool behavior_refines() const;

    // Returns the initials of Impl
    void impl_initials(Event::Set* out) const;

    // For a particular Impl initial event, enqueues a new refinement pair for
    // each Impl after.  Returns false if Spec cannot perform `initial` (meaning
    // the overall refinement check fails).
    bool enqueue_afters(Event initial, Set* enqueued, Set* pending) const;

    std::size_t hash() const;
    bool operator==(const RefinementPair<Model>& other) const;

  private:
    const NormalizedProcess* spec_;
    const Process* impl_;
};

}  // namespace

}  // namespace hst

namespace std {

template <typename Model>
struct hash<hst::RefinementPair<Model>>
{
    std::size_t operator()(const hst::RefinementPair<Model>& pair) const
    {
        return pair.hash();
    }
};

}  // namespace std

namespace hst {

template <typename Model>
bool
RefinementPair<Model>::behavior_refines() const
{
    typename Model::Behavior spec_behavior =
            Model::get_process_behavior(*spec_);
    typename Model::Behavior impl_behavior =
            Model::get_process_behavior(*impl_);
    return spec_behavior.refined_by(impl_behavior);
}

template <typename Model>
void
RefinementPair<Model>::impl_initials(Event::Set* out) const
{
    impl_->initials(out);
}

template <typename Model>
bool
RefinementPair<Model>::enqueue_afters(Event initial, Set* enqueued,
                                      Set* pending) const
{
    const NormalizedProcess* spec_after =
            initial == Event::tau() ? spec_ : spec_->after(initial);
    if (!spec_after) {
        // The spec cannot perform this event, so there's no outgoing edge for
        // the refinement check.
        return false;
    }

    // Otherwise we need to create a new refinement pair for the single after of
    // spec and all of the afters of impl.
    Process::Set impl_afters;
    impl_->afters(initial, &impl_afters);
    for (const Process* impl_after : impl_afters) {
        RefinementPair pair(spec_after, impl_after);
        bool added = enqueued->insert(pair).second;
        if (added) {
            pending->insert(pair);
        }
    }
    return true;
}

template <typename Model>
std::size_t
RefinementPair<Model>::hash() const
{
    static hash_scope refinement;
    return hasher(refinement).add(spec_).add(impl_).value();
}

template <typename Model>
bool
RefinementPair<Model>::operator==(const RefinementPair<Model>& other) const
{
    return spec_ == other.spec_ && impl_ == other.impl_;
}

template <typename Model>
bool
RefinementChecker<Model>::refines(const NormalizedProcess* spec,
                                  const Process* impl) const
{
    typename RefinementPair<Model>::Set enqueued;
    typename RefinementPair<Model>::Set queue;

    RefinementPair<Model> root(spec, impl);
    enqueued.insert(root);
    queue.insert(root);

    while (!queue.empty()) {
        typename RefinementPair<Model>::Set pending;
        for (const RefinementPair<Model>& pair : queue) {
            if (!pair.behavior_refines()) {
                // TODO: Construct a counterexample
                return false;
            }

            Event::Set initials;
            pair.impl_initials(&initials);
            for (const Event& initial : initials) {
                if (!pair.enqueue_afters(initial, &enqueued, &pending)) {
                    // TODO: Construct a counterexample
                    return false;
                }
            }
        }

        std::swap(queue, pending);
    }

    return true;
}

template class RefinementChecker<Traces>;

}  // namespace hst

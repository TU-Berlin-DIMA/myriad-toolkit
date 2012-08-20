x Add a `RecordRangeFilter` -- a multi-attribute alternative which works on a per-record basis for the invertible hydrators.
x Add a `RecordRangePredicate` template type (the single argument of the `RecordRangeFilter` function object).
x Add a `RecordRangePredicateBinder` -- a generic template component that binds values to `RecordRangePredicate` predicates (we need two flavors: for scalar fields of another record and for (potentially conditional -- how to handle that?!?!) probabilities.
x Integrate mandatory cardinality property in the base RecordMeta class.
x Add a `RecordRangePredicateBinderChain` -- may be directly integrated into the `RecordRangePredicateBinder` type. 
x Implement the const_range_provider.
x Implement the context_field_range_provider.
x Implement the const_value_provider.
x Implement the clustered_value_provider.
x Implement the const_value_provider.
x Implement the callback_value_provider.
x Implement the context_field_value_provider (the value_getter components can be refactored/reused here).
x Implement the random_value_provider.
x Implement the clustered_reference_provider.
x Implement the random_reference_provider.
x Merge conditional_randomized_hydrator, simple_randomized_hydrator, simple_clustered_hydrator, const_hydrator into a new generic type field_setter.
x Merge clustered_reference_hydrator, reference_hydrator, referenced_record_hydrator into a new generic type reference_setter.
x Adapt the XML prototype compiler to use the new reference_setter and field_setter components.

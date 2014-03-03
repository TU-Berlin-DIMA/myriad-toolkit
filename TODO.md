* Add a `RecordRangeFilter` -- a multi-attribute alternative which works on a per-record basis for the invertible hydrators.
* Add a `RecordRangePredicate` template type (the single argument of the `RecordRangeFilter` function object).
* Add a `RecordRangePredicateBinder` -- a generic template component that binds values to `RecordRangePredicate` predicates (we need two flavors: for scalar fields of another record and for (potentially conditional -- how to handle that?!?!) probabilities.
* Integrate mandatory cardinality property in the base RecordMeta class.
* Add a `RecordRangePredicateBinderChain` -- may be directly integrated into the `RecordRangePredicateBinder` type.
* Implement the const_range_provider.
* Implement the context_field_range_provider.
* Implement the const_value_provider.
* Implement the clustered_value_provider.
* Implement the const_value_provider.
* Implement the callback_value_provider.
* Implement the context_field_value_provider (the value_getter components can be refactored/reused here).
* Implement the random_value_provider.
* Implement the clustered_reference_provider.
* Implement the random_reference_provider.
* Merge conditional_randomized_hydrator, simple_randomized_hydrator, simple_clustered_hydrator, const_hydrator into a new generic type field_setter.
* Merge clustered_reference_hydrator, reference_hydrator, referenced_record_hydrator into a new generic type reference_setter.
* Adapt the XML prototype compiler to use the new reference_setter and field_setter components.

x Add a `RecordRangeFilter` -- a multi-attribute alternative which works on a per-record basis for the invertible hydrators.
* Add a `RecordRangePredicate` template type (the single argument of the `RecordRangeFilter` function object).
* Add a `RecordRangePredicateBinder` -- a generic template component that binds values to `RecordRangePredicate` predicates (we need two flavors: for scalar fields of another record and for (potentially conditional -- how to handle that?!?!) probabilities.
* Add a `RecordRangePredicateBinderChain` -- may be directly integrated into the `RecordRangePredicateBinder` type. 
* Adapt the XML prototype compiler to use the new components.
  
namespace std
{
  template <typename _Type>
    struct __type_identity
    { using type = _Type; };

  template<typename>
    struct __is_array_unknown_bounds;

  template <typename _Tp>
    constexpr int __is_complete_or_unbounded(__type_identity<_Tp>)
    { return {}; }

  template <typename _TypeIdentity,
      typename _NestedType = typename _TypeIdentity::type>
    constexpr int __is_complete_or_unbounded(_TypeIdentity)
    { return {}; }


  template<typename _Tp>
    struct is_trivial
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}));
    };
}

#pragma once

#include <engine/spin/Interaction_Traits.hpp>
#include <engine/spin/Interaction_Wrapper.hpp>

#include <tuple>

namespace Engine
{

namespace Spin
{

namespace Accessor
{

template<typename T>
using Energy_Single_Spin = typename T::Energy_Single_Spin;

template<typename T>
using Energy = typename T::Energy;

template<typename T>
using Energy_Total = typename T::Energy_Total;

template<typename T>
using Gradient = typename T::Gradient;

template<typename T>
using Hessian = typename T::Hessian;

} // namespace Accessor

namespace Functor
{

template<template<class> class FunctorAccessor, typename... InteractionTypes>
auto tuple_dispatch( std::tuple<Interaction::InteractionWrapper<InteractionTypes>...> & interactions )
    -> std::tuple<FunctorAccessor<InteractionTypes>...>
{
    return std::apply(
        []( Interaction::InteractionWrapper<InteractionTypes> &... interaction )
        { return std::make_tuple( interaction.template make_functor<FunctorAccessor>()... ); },
        interactions );
};

template<typename... Functors, typename... Args>
void apply( std::tuple<Functors...> functors, Args &&... args )
{
    if constexpr( sizeof...( Functors ) == 0 )
        return;
    else
        std::apply(
            [args = std::tuple<Args...>( std::forward<Args>( args )... )]( auto &... functor ) -> void
            { ( ..., std::apply( functor, args ) ); },
            functors );
}

template<typename... Functors, typename ReturnType, typename... Args>
auto apply_reduce( std::tuple<Functors...> functors, ReturnType zero, Args &&... args ) -> ReturnType
{
    return std::apply(
        [zero, args = std::tuple<Args...>( std::forward<Args>( args )... )]( auto &... functor ) -> ReturnType
        { return ( zero + ... + std::apply( functor, args ) ); },
        functors );
}

template<typename state_t, typename ReturnType, typename... Functors>
struct transform_op
{
    template<typename IndexTuple>
    auto operator()( const IndexTuple & index ) -> ReturnType
    {
        return std::apply(
            [&index, &state = this->state_ref, &zero = this->zero_repr]( Functors &... f ) -> ReturnType
            {
                return (
                    zero + ... + [&state, &zero, &index]( Functors & functor )
                        -> decltype( functor( std::get<typename Functors::Interaction::Index>( index ), state ) )
                    {
                        if( Functors::Interaction::is_contributing( functor.data, functor.cache ) )
                            return functor( std::get<typename Functors::Interaction::Index>( index ), state );
                        else
                            return zero;
                    }( f ) );
            },
            functors );
    };

    constexpr transform_op( std::tuple<Functors...> && functors, ReturnType zero, const state_t & state ) noexcept(
        std::is_nothrow_move_constructible_v<std::tuple<Functors...>>
        && std::is_nothrow_copy_constructible_v<ReturnType> )
            : functors( functors ), state_ref( state ), zero_repr( zero ){};

private:
    std::tuple<Functors...> functors;
    const state_t & state_ref;
    ReturnType zero_repr;
};

template<typename state_t, typename UnaryOp, typename... Functors>
struct for_each_op
{
    template<typename IndexTuple>
    auto operator()( const IndexTuple & index ) -> void
    {
        std::apply(
            [&index, &state = state_ref, &unary_op = unary_op_ref]( Functors &... functor ) -> void
            {
                ( ...,
                  [&state, &index, &unary_op]( Functors & functor ) -> void
                  {
                      if( Functors::Interaction::is_contributing( functor.data, functor.cache ) )
                          functor( std::get<typename Functors::Interaction::Index>( index ), state, unary_op );
                  }( functor ) );
            },
            functors );
    };

    constexpr for_each_op( std::tuple<Functors...> && functors, const state_t & state, UnaryOp & unary_op ) noexcept(
        std::is_nothrow_move_constructible_v<std::tuple<Functors...>> )
            : functors( functors ), state_ref( state ), unary_op_ref( unary_op ){};

private:
    std::tuple<Functors...> functors;
    const state_t & state_ref;
    UnaryOp & unary_op_ref;
};

} // namespace Functors

} // namespace Spin

} // namespace Engine
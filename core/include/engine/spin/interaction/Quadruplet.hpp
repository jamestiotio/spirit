#pragma once
#ifndef SPIRIT_CORE_ENGINE_INTERACTION_QUADRUPLET_HPP
#define SPIRIT_CORE_ENGINE_INTERACTION_QUADRUPLET_HPP

#include <engine/Indexing.hpp>
#include <engine/spin/interaction/Functor_Prototpyes.hpp>

#include <vector>

namespace Engine
{

namespace Spin
{

namespace Interaction
{

struct Quadruplet
{
    using state_t = vectorfield;

    struct Data
    {
        quadrupletfield quadruplets;
        scalarfield magnitudes;

        Data( quadrupletfield quadruplets, scalarfield magnitudes )
                : quadruplets( std::move( quadruplets ) ), magnitudes( std::move( magnitudes ) ){};
    };

    static bool valid_data( const Data & data )
    {
        return data.quadruplets.size() == data.magnitudes.size();
    };

    struct Cache
    {
        const ::Data::Geometry * geometry{};
        const intfield * boundary_conditions{};
    };

    static bool is_contributing( const Data & data, const Cache & )
    {
        return !data.quadruplets.empty();
    }

    struct IndexType
    {
        int ispin, jspin, kspin, lspin, iquad;
    };

    using Index = std::vector<IndexType>;

    using Energy   = Functor::Local::Energy_Functor<Quadruplet>;
    using Gradient = Functor::Local::Gradient_Functor<Quadruplet>;
    using Hessian  = Functor::Local::Hessian_Functor<Quadruplet>;

    static std::size_t Sparse_Hessian_Size_per_Cell( const Data &, const Cache & )
    {
        return 0;
    };

    // Calculate the total energy for a single spin to be used in Monte Carlo.
    //      Note: therefore the energy of pairs is weighted x2 and of quadruplets x4.
    using Energy_Single_Spin       = Functor::Local::Energy_Single_Spin_Functor<Energy, 4>;

    // Interaction name as string
    static constexpr std::string_view name = "Quadruplet";

    static constexpr bool local = true;

    template<typename IndexVector>
    static void applyGeometry(
        const ::Data::Geometry & geometry, const intfield & boundary_conditions, const Data & data, Cache & cache,
        IndexVector & indices )
    {
        using Indexing::idx_from_pair;

        for( int iquad = 0; iquad < data.quadruplets.size(); ++iquad )
        {
            const auto & quad = data.quadruplets[iquad];

            const int i = quad.i;
            const int j = quad.j;
            const int k = quad.k;
            const int l = quad.l;

            const auto & d_j = quad.d_j;
            const auto & d_k = quad.d_k;
            const auto & d_l = quad.d_l;

            for( unsigned int icell = 0; icell < geometry.n_cells_total; ++icell )
            {
                int ispin = i + icell * geometry.n_cell_atoms;
                int jspin = idx_from_pair(
                    ispin, boundary_conditions, geometry.n_cells, geometry.n_cell_atoms, geometry.atom_types,
                    { i, j, d_j } );
                int kspin = idx_from_pair(
                    ispin, boundary_conditions, geometry.n_cells, geometry.n_cell_atoms, geometry.atom_types,
                    { i, k, d_k } );
                int lspin = idx_from_pair(
                    ispin, boundary_conditions, geometry.n_cells, geometry.n_cell_atoms, geometry.atom_types,
                    { i, l, d_l } );

                if( jspin < 0 || kspin < 0 || lspin < 0 )
                    continue;

                std::get<Index>( indices[ispin] ).emplace_back( IndexType{ ispin, jspin, kspin, lspin, (int)iquad } );
                std::get<Index>( indices[jspin] ).emplace_back( IndexType{ jspin, ispin, kspin, lspin, (int)iquad } );
                std::get<Index>( indices[kspin] ).emplace_back( IndexType{ kspin, lspin, ispin, jspin, (int)iquad } );
                std::get<Index>( indices[lspin] ).emplace_back( IndexType{ lspin, kspin, ispin, jspin, (int)iquad } );
            }
        }

        cache.geometry            = &geometry;
        cache.boundary_conditions = &boundary_conditions;
    };
};

template<>
inline scalar Quadruplet::Energy::operator()( const Index & index, const vectorfield & spins ) const
{
    return std::transform_reduce(
        begin( index ), end( index ), scalar( 0.0 ), std::plus<scalar>{},
        [this, &spins]( const Quadruplet::IndexType & idx ) -> scalar
        {
            const auto & [ispin, jspin, kspin, lspin, iquad] = idx;
            return -0.25 * data.magnitudes[iquad] * ( spins[ispin].dot( spins[jspin] ) )
                   * ( spins[kspin].dot( spins[lspin] ) );
        } );
}

template<>
inline Vector3 Quadruplet::Gradient::operator()( const Index & index, const vectorfield & spins ) const
{
    return std::transform_reduce(
        begin( index ), end( index ), Vector3{ 0.0, 0.0, 0.0 }, std::plus<Vector3>{},
        [this, &spins]( const Quadruplet::IndexType & idx ) -> Vector3
        {
            const auto & [ispin, jspin, kspin, lspin, iquad] = idx;
            return spins[jspin] * ( -data.magnitudes[iquad] * ( spins[kspin].dot( spins[lspin] ) ) );
        } );
}

template<>
template<typename F>
void Quadruplet::Hessian::operator()( const Index & index, const vectorfield & spins, F & f ) const {
    // TODO
};

} // namespace Interaction

} // namespace Spin

} // namespace Engine

#endif

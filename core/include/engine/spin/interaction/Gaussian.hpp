#pragma once
#ifndef SPIRIT_CORE_ENGINE_INTERACTION_GAUSSIAN_HPP
#define SPIRIT_CORE_ENGINE_INTERACTION_GAUSSIAN_HPP

#include <engine/spin/interaction/Functor_Prototpyes.hpp>

#include <Eigen/Dense>

#include <optional>

namespace Engine
{

namespace Spin
{

namespace Interaction
{

/*
The Gaussian Hamiltonian is meant for testing purposes and demonstrations. Spins do not interact.
A set of gaussians is summed with weight-factors so as to create an arbitrary energy landscape.
E = sum_i^N a_i exp( -l_i^2(m)/(2sigma_i^2) ) where l_i(m) is the distance of m to the gaussian i,
    a_i is the gaussian amplitude and sigma_i the width
*/
struct Gaussian
{
    using state_t = vectorfield;

    struct Data
    {
        // Parameters of the energy landscape
        scalarfield amplitude;
        scalarfield width;
        vectorfield center;

        Data( scalarfield amplitude, scalarfield width, vectorfield center )
                : amplitude( std::move( amplitude ) ), width( std::move( width ) ), center( std::move( center ) ){};
    };

    static bool valid_data( const Data & data )
    {
        return ( data.amplitude.size() == data.width.size() && data.amplitude.size() == data.center.size() );
    };

    struct Cache
    {
        std::size_t n_gaussians;
    };

    static bool is_contributing( const Data & data, const Cache & )
    {
        return !data.amplitude.empty();
    };

    typedef int IndexType;

    using Index = std::optional<int>;

    using Energy   = Functor::Local::Energy_Functor<Gaussian>;
    using Gradient = Functor::Local::Gradient_Functor<Gaussian>;
    using Hessian  = Functor::Local::Hessian_Functor<Gaussian>;

    static std::size_t Sparse_Hessian_Size_per_Cell( const Data & data, const Cache & )
    {
        return 9 * data.amplitude.size();
    };

    // Calculate the total energy for a single spin
    using Energy_Single_Spin = Functor::Local::Energy_Single_Spin_Functor<Energy, 1>;

    // Interaction name as string
    static constexpr std::string_view name = "Gaussian";

    template<typename IndexVector>
    static void
    applyGeometry( const ::Data::Geometry & geometry, const intfield &, const Data &, Cache &, IndexVector & indices )
    {
        for( int icell = 0; icell < geometry.n_cells_total; ++icell )
        {
            for( int ibasis = 0; ibasis < geometry.n_cell_atoms; ++ibasis )
            {
                const int ispin                   = icell * geometry.n_cell_atoms + ibasis;
                std::get<Index>( indices[ispin] ) = ispin;
            };
        }
    }
};

template<>
inline scalar Gaussian::Energy::operator()( const Index & index, const vectorfield & spins ) const
{
    scalar result = 0;

    if( !index.has_value() )
        return result;

    const int ispin = *index;

    for( unsigned int igauss = 0; igauss < data.amplitude.size(); ++igauss )
    {
        // Distance between spin and gaussian center
        scalar l = 1 - data.center[igauss].dot( spins[ispin] );
        result += data.amplitude[igauss] * std::exp( -l * l / ( 2.0 * data.width[igauss] * data.width[igauss] ) );
    };
    return result;
}

template<>
inline Vector3 Gaussian::Gradient::operator()( const Index & index, const vectorfield & spins ) const
{
    Vector3 result = Vector3::Zero();

    if( !index.has_value() )
        return result;

    const int ispin = *index;
    // Calculate gradient
    for( unsigned int i = 0; i < data.amplitude.size(); ++i )
    {
        // Scalar product of spin and gaussian center
        scalar l = 1 - data.center[i].dot( spins[ispin] );
        // Prefactor
        scalar prefactor = data.amplitude[i] * std::exp( -l * l / ( 2.0 * data.width[i] * data.width[i] ) ) * l
                           / ( data.width[i] * data.width[i] );
        // Gradient contribution
        result += prefactor * data.center[i];
    }
    return result;
}

template<>
template<typename F>
void Gaussian::Hessian::operator()( const Index & index, const vectorfield & spins, F & f ) const
{
    if( !index.has_value() )
        return;

    const int ispin = *index;
    // Calculate Hessian
    for( unsigned int igauss = 0; igauss < data.amplitude.size(); ++igauss )
    {
        // Distance between spin and gaussian center
        scalar l = 1 - data.center[igauss].dot( spins[ispin] );
        // Prefactor for all alpha, beta
        scalar prefactor
            = data.amplitude[igauss] * std::exp( -std::pow( l, 2 ) / ( 2.0 * std::pow( data.width[igauss], 2 ) ) )
              / std::pow( data.width[igauss], 2 ) * ( std::pow( l, 2 ) / std::pow( data.width[igauss], 2 ) - 1 );
        // Effective Field contribution
        for( std::uint8_t alpha = 0; alpha < 3; ++alpha )
        {
            for( std::uint8_t beta = 0; beta < 3; ++beta )
            {
                std::size_t i = 3 * ispin + alpha;
                std::size_t j = 3 * ispin + beta;
                f( i, j, prefactor * data.center[igauss][alpha] * data.center[igauss][beta] );
            }
        }
    }
}

} // namespace Interaction

} // namespace Spin

} // namespace Engine

#endif

#pragma once
#ifndef SPIRIT_CORE_ENGINE_INTERACTION_HAMILTONIAN_HPP
#define SPIRIT_CORE_ENGINE_INTERACTION_HAMILTONIAN_HPP

#include <Spirit/Hamiltonian.h>
#include <Spirit/Spirit_Defines.h>
#include <data/Geometry.hpp>
#include <engine/FFT.hpp>
#include <engine/Hamiltonian_Defines.hpp>
#include <engine/Vectormath_Defines.hpp>
#include <utility/Span.hpp>

#include <memory>
#include <random>
#include <vector>

namespace Engine
{

class Hamiltonian;

// forward declaration of interactions for following friend declaration
namespace Interaction
{

using triplet = Eigen::Triplet<scalar>;

// common interaction base class
class ABC;

// CRTP class for shared implementations that need knowledge of the specific child class
template<class Derived>
class Base;

// Interaction classes
class Gaussian;
class Zeeman;
class Anisotropy;
class Cubic_Anisotropy;
class Exchange;

void setOwnerPtr( ABC & interaction, Hamiltonian * hamiltonian ) noexcept;

} // namespace Interaction

enum class HAMILTONIAN_CLASS
{
    GENERIC    = SPIRIT_HAMILTONIAN_CLASS_GENERIC,
    GAUSSIAN   = SPIRIT_HAMILTONIAN_CLASS_GAUSSIAN,
    HEISENBERG = SPIRIT_HAMILTONIAN_CLASS_HEISENBERG,
};

static constexpr std::string_view hamiltonianClassName( HAMILTONIAN_CLASS cls )
{
    switch( cls )
    {
        case HAMILTONIAN_CLASS::GENERIC: return "Generic";
        case HAMILTONIAN_CLASS::GAUSSIAN: return "Gaussian";
        case HAMILTONIAN_CLASS::HEISENBERG: return "Heisenberg";
        default: return "Unknown";
    };
}

// Forward declaration of DDI_Method
enum class DDI_Method
{
    FFT    = SPIRIT_DDI_METHOD_FFT,
    FMM    = SPIRIT_DDI_METHOD_FMM,
    Cutoff = SPIRIT_DDI_METHOD_CUTOFF,
    None   = SPIRIT_DDI_METHOD_NONE
};

/*
    The Heisenberg Hamiltonian using Pairs contains all information on the interactions between spins.
    The information is presented in pair lists and parameter lists in order to easily e.g. calculate the energy of the
   system via summation. Calculations are made on a per-pair basis running over all pairs.
*/
class Hamiltonian
{
    // Marked as friend classes as an alternative to having a proper accessor for geometry & boundary_conditions.
    // This might also prove to be more flexible and makes sense semantically because interactions and Hamiltonian are
    // so intimately linked
    friend class Interaction::ABC;
    template<class Derived>
    friend class Interaction::Base;
    // Interaction classes
    friend class Interaction::Gaussian;
    friend class Interaction::Zeeman;
    friend class Interaction::Anisotropy;
    friend class Interaction::Cubic_Anisotropy;
    friend class Interaction::Exchange;

public:
    friend void swap( Hamiltonian & first, Hamiltonian & second ) noexcept
    {
        using std::swap;
        if( &first == &second )
        {
            return;
        }
        swap( first.geometry, second.geometry );
        swap( first.boundary_conditions, second.boundary_conditions );
        swap( first.name_update_paused, second.name_update_paused );
        swap( first.hamiltonian_class, second.hamiltonian_class );
        swap( first.class_name, second.class_name );

        // interactions legacy block
        swap( first.dmi_shell_magnitudes, second.dmi_shell_magnitudes );
        swap( first.dmi_shell_chirality, second.dmi_shell_chirality );
        swap( first.dmi_pairs_in, second.dmi_pairs_in );
        swap( first.dmi_magnitudes_in, second.dmi_magnitudes_in );
        swap( first.dmi_normals_in, second.dmi_normals_in );
        swap( first.quadruplets, second.quadruplets );
        swap( first.quadruplet_magnitudes, second.quadruplet_magnitudes );
        // ddi legacy block
        swap( first.ddi_method, second.ddi_method );
        swap( first.ddi_n_periodic_images, second.ddi_n_periodic_images );
        swap( first.ddi_pb_zero_padding, second.ddi_pb_zero_padding );
        swap( first.ddi_cutoff_radius, second.ddi_cutoff_radius );
        swap( first.fft_plan_spins, second.fft_plan_spins );
        swap( first.fft_plan_reverse, second.fft_plan_reverse );
        swap( first.n_inter_sublattice, second.n_inter_sublattice );
        swap( first.sublattice_size, second.sublattice_size );
        swap( first.n_cells_padded, second.n_cells_padded );
        swap( first.transformed_dipole_matrices, second.transformed_dipole_matrices );
        swap( first.dipole_matrices, second.dipole_matrices );
        swap( first.save_dipole_matrices, second.save_dipole_matrices );
        swap( first.dipole_stride, second.dipole_stride );
        swap( first.spin_stride, second.spin_stride );
        swap( first.it_bounds_pointwise_mult, second.it_bounds_pointwise_mult );
        swap( first.it_bounds_write_gradients, second.it_bounds_write_gradients );
        swap( first.it_bounds_write_spins, second.it_bounds_write_spins );
        swap( first.it_bounds_write_dipole, second.it_bounds_write_dipole );
        // energy_contributions_per_spin legacy block
        swap( first.energy_contributions_per_spin, second.energy_contributions_per_spin );
        swap( first.idx_gaussian, second.idx_gaussian );
        swap( first.idx_zeeman, second.idx_zeeman );
        swap( first.idx_anisotropy, second.idx_anisotropy );
        swap( first.idx_cubic_anisotropy, second.idx_cubic_anisotropy );
        swap( first.idx_exchange, second.idx_exchange );
        swap( first.idx_dmi, second.idx_dmi );
        swap( first.idx_quadruplet, second.idx_quadruplet );
        swap( first.idx_ddi, second.idx_ddi );

        swap( first.interactions, second.interactions );
        swap( first.active_interactions_size, second.active_interactions_size );
        swap( first.common_interactions_size, second.common_interactions_size );

        swap( first.prng, second.prng );
        swap( first.distribution_int, second.distribution_int );
        swap( first.delta, second.delta );

        for( const auto & interaction : first.interactions )
            Interaction::setOwnerPtr( *interaction, &first );

        for( const auto & interaction : second.interactions )
            Interaction::setOwnerPtr( *interaction, &second );
    }

    Hamiltonian(
        std::shared_ptr<Data::Geometry> geometry, const intfield & boundary_conditions,
        const Data::VectorPairfieldData & dmi,
        const Data::QuadrupletfieldData & quadruplet, Engine::DDI_Method ddi_method, const Data::DDI_Data & ddi_data );

    Hamiltonian(
        std::shared_ptr<Data::Geometry> geometry, const intfield & boundary_conditions,
        const scalarfield & dmi_shell_magnitudes, int dm_chirality,
        const Data::QuadrupletfieldData & quadruplet, Engine::DDI_Method ddi_method, const Data::DDI_Data & ddi_data );

    Hamiltonian( std::shared_ptr<Data::Geometry> geometry, intfield boundary_conditions );

    Hamiltonian() = default;
    // rule of five, because the Hamiltonian owns the interactions
    Hamiltonian( const Hamiltonian & other );
    // using the copy-and-swap idiom for brevity. Should be implemented more efficiently once the refactor is finished.
    Hamiltonian & operator=( Hamiltonian other )
    {
        swap( *this, other );
        return *this;
    };

    Hamiltonian( Hamiltonian && other ) noexcept : Hamiltonian()
    {
        swap( *this, other );
    };

    Hamiltonian & operator=( Hamiltonian && other ) noexcept
    {
        swap( *this, other );
        return *this;
    };

    ~Hamiltonian() = default;

    Utility::Span<const std::unique_ptr<Interaction::ABC>> getActiveInteractions() const
    {
        return Utility::Span( interactions.begin(), active_interactions_size );
    }

    std::size_t getActiveInteractionsSize() const
    {
        return active_interactions_size;
    };

    Utility::Span<const std::unique_ptr<Interaction::ABC>> getInactiveInteractions() const
    {
        return Utility::Span( interactions.begin() + active_interactions_size, interactions.end() );
    }

    std::size_t getInactiveInteractionsSize() const
    {
        return interactions.size() - active_interactions_size;
    };

    const auto & getInteractions() const
    {
        return interactions;
    };

    std::size_t getInteractionsSize() const
    {
        return interactions.size();
    };

    /*
     * Update the internal state of the interactions.
     * This needs to be done every time the parameters are changed, in case an energy
     * contribution is now non-zero or vice versa.
     * The interactions know when to trigger this if the parameters are updated for them directly,
     * the manual update should only be neccessary if the geometry or boundary_conditions are changed.
     */
    void updateInteractions();
    void updateActiveInteractions();

    // old mechanism
    void Update_Energy_Contributions();

    /*
     * update functions for when the geometry or an Interaction has been changed,
     * these serve as an alternative to more rigid accessors
     */
    void onInteractionChanged()
    {
        Update_Energy_Contributions();
    };
    void onGeometryChanged()
    {
        updateInteractions();
    };

    void Hessian( const vectorfield & spins, MatrixX & hessian );
    void Sparse_Hessian( const vectorfield & spins, SpMatrixX & hessian );

    void Gradient( const vectorfield & spins, vectorfield & gradient );
    void Gradient_and_Energy( const vectorfield & spins, vectorfield & gradient, scalar & energy );

    void Energy_Contributions_per_Spin( const vectorfield & spins, Data::vectorlabeled<scalarfield> & contributions );
    Data::vectorlabeled<scalar> Energy_Contributions( const vectorfield & spins );

    // Calculate the total energy for a single spin to be used in Monte Carlo.
    //      Note: therefore the energy of pairs is weighted x2 and of quadruplets x4.
    scalar Energy_Single_Spin( int ispin, const vectorfield & spins );

    scalar Energy( const vectorfield & spins );

    void Gradient_FD( const vectorfield & spins, vectorfield & gradient );
    void Hessian_FD( const vectorfield & spins, MatrixX & hessian );
    std::size_t Number_of_Interactions();

    // Hamiltonian name as string
    void pauseUpdateName()
    {
        name_update_paused = true;
    };

    void unpauseUpdateName()
    {
        name_update_paused = false;
    };

    void updateName();
    std::string_view Name() const;

    std::shared_ptr<Data::Geometry> geometry;
    intfield boundary_conditions;

private:
    std::vector<std::unique_ptr<Interaction::ABC>> interactions{};
    std::size_t active_interactions_size = 0;
    std::size_t common_interactions_size = 0;

public:
    // DMI
    scalarfield dmi_shell_magnitudes;
    int dmi_shell_chirality;
    pairfield dmi_pairs_in;
    scalarfield dmi_magnitudes_in;
    vectorfield dmi_normals_in;
    pairfield dmi_pairs;
    scalarfield dmi_magnitudes;
    vectorfield dmi_normals;
    // Dipole Dipole interaction
    DDI_Method ddi_method;
    intfield ddi_n_periodic_images;
    bool ddi_pb_zero_padding;
    //      ddi cutoff variables
    scalar ddi_cutoff_radius;
    pairfield ddi_pairs;
    scalarfield ddi_magnitudes;
    vectorfield ddi_normals;

    // ------------ Quadruplet Interactions ------------
    quadrupletfield quadruplets;
    scalarfield quadruplet_magnitudes;

    // ------------ Effective Field Functions ------------
    // Calculate the DMI effective field of a Spin Pair
    void Gradient_DMI( const vectorfield & spins, vectorfield & gradient );
    // Calculates the Dipole-Dipole contribution to the effective field of spin ispin within system s
    void Gradient_DDI( const vectorfield & spins, vectorfield & gradient );

    // Quadruplet
    void Gradient_Quadruplet( const vectorfield & spins, vectorfield & gradient );

    // ------------ Energy Functions ------------
    // Getters for Indices of the energy vector
    inline int Idx_Gaussian()
    {
        return idx_gaussian;
    }
    inline int Idx_Zeeman()
    {
        return idx_zeeman;
    };
    inline int Idx_Anisotropy()
    {
        return idx_anisotropy;
    };
    inline int Idx_Cubic_Anisotropy()
    {
        return idx_cubic_anisotropy;
    };
    inline int Idx_Exchange()
    {
        return idx_exchange;
    };
    inline int Idx_DMI()
    {
        return idx_dmi;
    };
    inline int Idx_DDI()
    {
        return idx_ddi;
    };
    inline int Idx_Quadruplet()
    {
        return idx_quadruplet;
    };

    // Calculate the DMI energy of a Spin System
    void E_DMI( const vectorfield & spins, scalarfield & Energy );
    // Calculate the Dipole-Dipole energy
    void E_DDI( const vectorfield & spins, scalarfield & Energy );
    // Calculate the Quadruplet energy
    void E_Quadruplet( const vectorfield & spins, scalarfield & Energy );

    Interaction::ABC * getInteraction( std::string_view name );
    std::size_t deleteInteraction( std::string_view name );

    template<class T, typename... Args>
    T & setInteraction( Args &&... args );

    template<class T>
    const T * getInteraction() const;

    template<class T>
    T * getInteraction();

    template<class T>
    bool hasInteraction();

    template<class T>
    std::size_t deleteInteraction();

private:
    // common and uncommon interactions partition the active interactions
    Utility::Span<const std::unique_ptr<Interaction::ABC>> getCommonInteractions() const
    {
        return Utility::Span( interactions.begin(), common_interactions_size );
    };

    Utility::Span<const std::unique_ptr<Interaction::ABC>> getUncommonInteractions() const
    {
        return Utility::Span(
            interactions.begin() + common_interactions_size, active_interactions_size - common_interactions_size );
    };

    int idx_gaussian, idx_zeeman, idx_anisotropy, idx_cubic_anisotropy, idx_exchange, idx_dmi, idx_ddi, idx_quadruplet;
    void Gradient_DDI_Cutoff( const vectorfield & spins, vectorfield & gradient );
    void Gradient_DDI_Direct( const vectorfield & spins, vectorfield & gradient );
    void Gradient_DDI_FFT( const vectorfield & spins, vectorfield & gradient );
    void E_DDI_Direct( const vectorfield & spins, scalarfield & Energy );
    void E_DDI_Cutoff( const vectorfield & spins, scalarfield & Energy );
    void E_DDI_FFT( const vectorfield & spins, scalarfield & Energy );

    Data::vectorlabeled<scalarfield> energy_contributions_per_spin;

    std::mt19937 prng;
    std::uniform_int_distribution<int> distribution_int;
    scalar delta = 1e-3;

    // naming mechanism for compatibility with the named subclasses architecture
    bool name_update_paused             = false;
    HAMILTONIAN_CLASS hamiltonian_class = HAMILTONIAN_CLASS::GENERIC;
    std::string_view class_name{ hamiltonianClassName( HAMILTONIAN_CLASS::GENERIC ) };

    static constexpr int common_spin_order = 2;
    // Preparations for DDI-Convolution Algorithm
    void Prepare_DDI();
    void Clean_DDI();

    // Plans for FT / rFT
    FFT::FFT_Plan fft_plan_spins;
    FFT::FFT_Plan fft_plan_reverse;

    field<FFT::FFT_cpx_type> transformed_dipole_matrices;

    bool save_dipole_matrices = false;
    field<FFT::FFT_real_type> dipole_matrices;

    // Number of inter-sublattice contributions
    int n_inter_sublattice;
    // At which index to look up the inter-sublattice D-matrices
    field<int> inter_sublattice_lookup;

    // Lengths of padded system
    field<int> n_cells_padded;
    // Total number of padded spins per sublattice
    int sublattice_size;

    FFT::StrideContainer spin_stride;
    FFT::StrideContainer dipole_stride;

    // Calculate the FT of the padded D-matrics
    void FFT_Dipole_Matrices( FFT::FFT_Plan & fft_plan_dipole, int img_a, int img_b, int img_c );
    // Calculate the FT of the padded spins
    void FFT_Spins( const vectorfield & spins );

    // Bounds for nested for loops. Only important for the CUDA version
    field<int> it_bounds_pointwise_mult;
    field<int> it_bounds_write_gradients;
    field<int> it_bounds_write_spins;
    field<int> it_bounds_write_dipole;
};

} // namespace Engine

#endif

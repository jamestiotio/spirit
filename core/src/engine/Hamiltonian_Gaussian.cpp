#include <engine/Hamiltonian_Gaussian.hpp>
#include <engine/Vectormath.hpp>

using namespace Data;

namespace Engine
{
	Hamiltonian_Gaussian::Hamiltonian_Gaussian(
		std::vector<scalar> amplitude, std::vector<scalar> width, std::vector<Vector3> center
	) :
		Hamiltonian(std::vector<bool>{false, false, false}), amplitude(amplitude), width(width), center(center)
	{
		this->n_gaussians = amplitude.size();
	}

	void Hamiltonian_Gaussian::Update_Energy_Contributions()
	{
		this->energy_contributions_per_spin = { { "Gaussian", scalarfield(0) } };
	}

	void Hamiltonian_Gaussian::Hessian(const vectorfield & spins, MatrixX & hessian)
	{
		//int nos = spins.size();
		//for (int ispin = 0; ispin < nos; ++ispin)
		//{
		//	// Set Hessian to zero
		//	for (int alpha = 0; alpha < 3; ++alpha)
		//	{
		//		for (int beta = 0; beta < 3; ++beta)
		//		{
		//			int idx_h = ispin + alpha*nos + 3 * nos*(ispin + beta*nos);
		//			hessian[idx_h] = 0.0;
		//		}
		//	}
		//	// Calculate Hessian
		//	for (int i = 0; i < this->n_gaussians; ++i)
		//	{
		//		// Distance between spin and gaussian center
		//		scalar l = 1 - this->center[i].dot(spins[ispin]); //Utility::Manifoldmath::Dist_Greatcircle(this->center[i], n);
		//		// Scalar product of spin and gaussian center
		//		//scalar nc = 0;
		//		//for (int dim = 0; dim < 3; ++dim) nc += spins[ispin + dim*nos] * this->center[i][dim];
		//		// Prefactor for all alpha, beta
		//		scalar prefactor = this->amplitude[i] * std::exp(-std::pow(l, 2) / (2.0*std::pow(this->width[i], 2)))
		//			/ std::pow(this->width[i], 2)
		//			* (std::pow(l, 2) / std::pow(this->width[i], 2) - 1);
		//		// Effective Field contribution
		//		for (int alpha = 0; alpha < 3; ++alpha)
		//		{
		//			for (int beta = 0; beta < 3; ++beta)
		//			{
		//				int idx_h = ispin + alpha*nos + 3 * nos*(ispin + beta*nos);
		//				hessian[idx_h] += prefactor * this->center[i][alpha] * this->center[i][beta];
		//			}
		//		}
		//	}
		//}
	}

	void Hamiltonian_Gaussian::Effective_Field(const vectorfield & spins, vectorfield & field)
	{
		int nos = spins.size();

		for (int ispin = 0; ispin < nos; ++ispin)
		{
			// Set field to zero
			field[ispin] = { 0,0,0 };
			// Calculate field
			for (int i = 0; i < this->n_gaussians; ++i)
			{
				// Distance between spin and gaussian center
				scalar l = 1 - this->center[i].dot(spins[ispin]); //Utility::Manifoldmath::Dist_Greatcircle(this->center[i], n);
				// Scalar product of spin and gaussian center
				//scalar nc = 0;
				//for (int dim = 0; dim < 3; ++dim) nc += spins[ispin + dim*nos] * this->center[i][dim];
				// Prefactor
				scalar prefactor = this->amplitude[i] * std::exp(-std::pow(l, 2) / (2.0*std::pow(this->width[i], 2))) * l / std::pow(this->width[i],2);
				// Effective Field contribution
				field[ispin ] -= prefactor * this->center[i];
			}
		}
	}

	void Hamiltonian_Gaussian::Energy_Contributions_per_Spin(const vectorfield & spins, std::vector<std::pair<std::string, scalarfield>> & contributions)
	{
		int nos = spins.size();

		// Allocate if not already allocated
		if (this->energy_contributions_per_spin[0].second.size() != nos) this->energy_contributions_per_spin = { { "Gaussian", scalarfield(nos,0) } };

		// Set to zero
		for (auto& pair : energy_contributions_per_spin) Vectormath::fill(pair.second, 0);

		for (int i = 0; i < this->n_gaussians; ++i)
		{
			for (int ispin = 0; ispin < nos; ++ispin)
			{
				// Distance between spin and gaussian center
				scalar l = 1 - this->center[i].dot(spins[ispin]); //Utility::Manifoldmath::Dist_Greatcircle(this->center[i], n);
																  // Energy contribution
				this->energy_contributions_per_spin[0].second[ispin] += this->amplitude[i] * std::exp(-std::pow(l, 2) / (2.0*std::pow(this->width[i], 2)));
			}
		}
	}

	// Hamiltonian name as string
	static const std::string name = "Gaussian";
	const std::string& Hamiltonian_Gaussian::Name() { return name; }
}
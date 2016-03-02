/*
 * gmm.h
 *
 * Gaussian Mixture Model for continuous recognition and regression
 *
 * Contact:
 * - Jules Françoise <jules.francoise@ircam.fr>
 *
 * This code has been initially authored by Jules Françoise
 * <http://julesfrancoise.com> during his PhD thesis, supervised by Frédéric
 * Bevilacqua <href="http://frederic-bevilacqua.net>, in the Sound Music
 * Movement Interaction team <http://ismm.ircam.fr> of the
 * STMS Lab - IRCAM, CNRS, UPMC (2011-2015).
 *
 * Copyright (C) 2015 UPMC, Ircam-Centre Pompidou.
 *
 * This File is part of XMM.
 *
 * XMM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XMM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XMM.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef xmm_lib_gmm_h
#define xmm_lib_gmm_h

#include "../core/gaussian_distribution.h"
#include "../core/probabilistic_model.h"

namespace xmm
{
    /**
     * @defgroup GMM Gaussian Mixture Models
     */
    
    /**
     * @ingroup GMM
     * @class GMM
     * @brief Gaussian Mixture Models
     * @details Multivariate Gaussian Mixture Model. Supports Bimodal data and Gaussian Mixture Regression.
     * Can be either autonomous or a state of a HMM: defines observation probabilities for each state.
     */
    class GMM : public ProbabilisticModel {
    public:
        friend class HMM;
        friend class HierarchicalHMM;
        
        ///@cond DEVDOC
        static const int DEFAULT_NB_MIXTURE_COMPONENTS = 1;
        ///@endcond
        
        /**
         * @brief Iterator over the phrases of the training set.
         */
        typedef std::map<int, Phrase* >::iterator phrase_iterator;
        
        /**
         * @brief Iterator over Gaussian Mixture Components
         */
        typedef std::vector<GaussianDistribution>::iterator mixture_iterator;
        
#pragma mark -
#pragma mark === Public Interface ===
#pragma mark > Constructors
        /** @name Constructors */
        ///@{
        
        /**
         * @brief Constructor
         * @param flags Construction Flags: use 'BIMODAL' for use with Gaussian Mixture Regression.
         * @param trainingSet training set associated with the model
         * @param nbMixtureComponents number of mixture components
         * @param varianceOffset_relative offset added to the diagonal of covariances matrices (relative to data variance)
         * @param varianceOffset_absolute offset added to the diagonal of covariances matrices (minimum value)
         * @param covariance_mode covariance mode (full vs diagonal)
         */
        GMM(xmm_flags flags = NONE,
            TrainingSet *trainingSet=NULL,
            int nbMixtureComponents = DEFAULT_NB_MIXTURE_COMPONENTS,
            double varianceOffset_relative = GaussianDistribution::DEFAULT_VARIANCE_OFFSET_RELATIVE(),
            double varianceOffset_absolute = GaussianDistribution::DEFAULT_VARIANCE_OFFSET_ABSOLUTE(),
            GaussianDistribution::COVARIANCE_MODE covariance_mode = GaussianDistribution::FULL);
        
        /**
         * @brief Copy constructor
         * @param src Source GMM
         */
        GMM(GMM const& src);
        
        /**
         * @brief Assignment
         * @param src Source GMM
         */
        GMM& operator=(GMM const& src);
        
        /**
         * @brief Destructor
         */
        virtual ~GMM();
        
        ///@}
        
#pragma mark > Accessors
        /** @name Accessors */
        ///@{
        /**
         * @brief Get the number of Gaussian mixture Components
         * @return number of Gaussian mixture components
         */
        int get_nbMixtureComponents() const;
        
        /**
         * @brief Get Offset added to covariance matrices for convergence
         * @return Offset added to covariance matrices for convergence (relative to data variance)
         */
        double get_varianceOffset_relative() const;
        
        /**
         * @brief Get Offset added to covariance matrices for convergence
         * @return Offset added to covariance matrices for convergence (minimum value)
         */
        double get_varianceOffset_absolute() const;
        
        /**
         * @brief Set the number of mixture components of the model
         * @warning sets the model to be untrained.
         * @param nbMixtureComponents number of Gaussian Mixture Components
         * @throws invalid_argument if nbMixtureComponents is <= 0
         */
        void set_nbMixtureComponents(int nbMixtureComponents);
        
        /**
         * @brief Set the offset to add to the covariance matrices
         * @param varianceOffset_relative offset to add to the diagonal of covariance matrices (relative to data variance)
         * @param varianceOffset_absolute offset to add to the diagonal of covariance matrices (minimum value)
         * @throws invalid_argument if the covariance offset is <= 0
         */
        void set_varianceOffset(double varianceOffset_relative, double varianceOffset_absolute);
        
        /**
         * @brief get the current covariance mode
         */
        GaussianDistribution::COVARIANCE_MODE get_covariance_mode() const;
        
        /**
         * @brief set the covariance mode
         * @param covariance_mode target covariance mode
         */
        void set_covariance_mode(GaussianDistribution::COVARIANCE_MODE covariance_mode);
        
        ///@}
        
#pragma mark > Performance
        /** @name Performance */
        ///@{
        /**
         * @brief Initialize performance mode
         */
        void performance_init();
        
        /**
         * @brief Main Play function: performs recognition (unimodal mode) or regression (bimodal mode)
         * @details The predicted output is stored in the observation vector in bimodal mode
         * @param observation observation (must allocated to size 'dimension')
         * @return instantaneous likelihood
         */
        double performance_update(std::vector<float> const& observation);
        
        ///@}
        
#pragma mark > JSON I/O
        /** @name JSON I/O */
        ///@{
        /**
         * @brief Write to JSON Node
         * @return JSON Node containing model information and parameters
         */
        JSONNode to_json() const;
        
        /**
         * @brief Read from JSON Node
         * @details allocate model parameters and updates inverse Covariances
         * @param root JSON Node containing model information and parameters
         * @throws JSONException if the JSONNode has a wrong format
         */
        virtual void from_json(JSONNode root);
        
        ///@}
        
#pragma mark > Conversion & Extraction
        /** @name Conversion & Extraction */
        ///@{
        
        /**
         * @brief Convert to bimodal GMM in place
         * @param dimension_input dimension of the input modality
         * @throws runtime_error if the model is already bimodal
         * @throws out_of_range if the requested input dimension is too large
         */
        void make_bimodal(unsigned int dimension_input);
        
        /**
         * @brief Convert to unimodal GMM in place
         * @throws runtime_error if the model is already unimodal
         */
        void make_unimodal();
        
        /**
         * @brief extract a submodel with the given columns
         * @param columns columns indices in the target order
         * @throws runtime_error if the model is training
         * @throws out_of_range if the number or indices of the requested columns exceeds the current dimension
         * @return a GMM from the current model considering only the target columns
         */
        GMM extract_submodel(std::vector<unsigned int>& columns) const;
        
        /**
         * @brief extract the submodel of the input modality
         * @throws runtime_error if the model is training or if it is not bimodal
         * @return a unimodal GMM of the input modality from the current bimodal model
         */
        GMM extract_submodel_input() const;
        
        /**
         * @brief extract the submodel of the output modality
         * @throws runtime_error if the model is training or if it is not bimodal
         * @return a unimodal GMM of the output modality from the current bimodal model
         */
        GMM extract_submodel_output() const;
        
        /**
         * @brief extract the model with reversed input and output modalities
         * @throws runtime_error if the model is training or if it is not bimodal
         * @return a bimodal GMM  that swaps the input and output modalities
         */
        GMM extract_inverse_model() const;
        
        ///@}
        
#pragma mark -
#pragma mark === Public attributes ===
        /**
         * @brief Vector of Gaussian Mixture Components
         */
        std::vector<GaussianDistribution> components;
        
        /**
         * @brief Mixture Coefficients
         */
        std::vector<float> mixtureCoeffs;
        
        /**
         * @brief Beta probabilities: estimated likelihoods of each component
         */
        std::vector<double> beta;
        
    protected:
        ///@cond DEVDOC
#pragma mark -
#pragma mark === Protected Methods ===
#pragma mark > Utilities
        /** @name Utilities (protected) */
        ///@{
        /**
         * @brief Copy between 2 GMMs
         * @param dst Destination GMM
         * @param src Source GMM
         */
        using ProbabilisticModel::_copy;
        virtual void _copy(GMM *dst, GMM const& src);
        
        /**
         @brief Allocate model parameters
         */
        void allocate();
        
        /**
         * @brief Observation probability
         * @param observation observation vector (must be of size 'dimension')
         * @param mixtureComponent index of the mixture component. if unspecified or negative,
         * full mixture observation probability is computed
         * @return likelihood of the observation given the model
         * @throws out_of_range if the index of the Gaussian Mixture Component is out of bounds
         * @throws runtime_error if the Covariance Matrix is not invertible
         */
        double obsProb(const float* observation, int mixtureComponent=-1);
        
        /**
         * @brief Observation probability on the input modality
         * @param observation_input observation vector of the input modality (must be of size 'dimension_input')
         * @param mixtureComponent index of the mixture component. if unspecified or negative,
         * full mixture observation probability is computed
         * @return likelihood of the observation of the input modality given the model
         * @throws runtime_error if the model is not bimodal
         * @throws runtime_error if the Covariance Matrix of the input modality is not invertible
         */
        double obsProb_input(const float* observation_input, int mixtureComponent=-1);
        
        /**
         * @brief Observation probability for bimodal mode
         * @param observation_input observation vector of the input modality (must be of size 'dimension_input')
         * @param observation_output observation vector of the input output (must be of size 'dimension - dimension_input')
         * @param mixtureComponent index of the mixture component. if unspecified or negative,
         * full mixture observation probability is computed
         * @return likelihood of the observation of the input modality given the model
         * @throws runtime_error if the model is not bimodal
         * @throws runtime_error if the Covariance Matrix is not invertible
         */
        double obsProb_bimodal(const float* observation_input, const float* observation_output, int mixtureComponent=-1);
        
        ///@}
        
#pragma mark > Training
        /** @name Training (protected) */
        ///@{
        
        /**
         * @brief Initialize the means of the Gaussian components with a Biased K-means
         */
        void initMeansWithKMeans();
        
        /**
         * @brief Initialize the Covariances of the Gaussian components using a fully observed approximation
         */
        void initCovariances_fullyObserved();
        
        /**
         * @brief Initialize the EM Training Algorithm
         * @details Initializes the Gaussian Components from the first phrase
         * of the Training Set
         */
        virtual void train_EM_init();
        
        /**
         * @brief Update Function of the EM algorithm
         * @return likelihood of the data given the current parameters (E-step)
         */
        double train_EM_update();
        
        /**
         * @brief Initialize model parameters to default values.
         * @details Mixture coefficients are then equiprobable
         */
        virtual void initParametersToDefault();
        
        /**
         * @brief Normalize mixture coefficients
         */
        void normalizeMixtureCoeffs();
        
        /**
         * @brief Add offset to the diagonal of the covariance matrices
         * @details Guarantees convergence through covariance matrix invertibility
         */
        void addCovarianceOffset();
        
        /**
         * @brief Update inverse covariances of each Gaussian component
         * @throws runtime_error if one of the covariance matrices is not invertible
         */
        void updateInverseCovariances();
        
        ///@}
        
#pragma mark > Performance
        /** @name Performance (protected) */
        ///@{
        
        /**
         * @brief Compute likelihood and estimate components probabilities
         * @details If the model is bimodal, the likelihood is computed only on the input modality,
         * except if 'observation_output' is specified.
         * Updates the likelihood buffer used to smooth likelihoods.
         * @param observation observation vector (full size for unimodal, input modality for bimodal)
         * @param observation_output observation vector of the output modality
         */
        double likelihood(std::vector<float> const& observation,
                          std::vector<float> const& observation_output = null_vector_float);
        
        /**
         * @brief Compute Gaussian Mixture Regression
         * @details Estimates the output modality using covariance-based regression weighted by components' likelihood
         * @warning the function does not estimates the likelihoods, use 'likelihood' before performing
         * the regression.
         * @param observation_input observation vector of the input modality
         * @param predicted_output observation vector where the predicted observation for the output
         * modality is stored.
         */
        void regression(std::vector<float> const& observation_input,
                        std::vector<float>& predicted_output);
        
        ///@}
        
#pragma mark -
#pragma mark === Protected attributes ===
        /**
         * @brief Number of Gaussian Mixture Components
         */
        int nbMixtureComponents_;
        
        /**
         * @brief Offset Added to the diagonal of covariance matrices for convergence (Relative to Data Variance)
         */
        double varianceOffset_relative_;
        
        /**
         * @brief Offset Added to the diagonal of covariance matrices for convergence (minimum value)
         */
        double varianceOffset_absolute_;
        
        /**
         * @brief Covariance Mode
         */
        
        ///@endcond
    public:
        GaussianDistribution::COVARIANCE_MODE covariance_mode_;
    };
}

#endif

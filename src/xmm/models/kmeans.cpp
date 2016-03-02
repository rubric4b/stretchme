/*
 * kmeans.cpp
 *
 * K-Means clustering
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

#include "xmm/models/kmeans.h"
#include <limits>

#ifdef WIN32
static long random() {return rand();}
#endif
#define kmax(a,b) (((a) > (b)) ? (a) : (b))

#pragma mark -
#pragma mark === Public Interface ===
#pragma mark > Constructors
xmm::KMeans::KMeans(TrainingSet *trainingSet,
               int nbClusters)
: trainingSet(trainingSet),
  trainingInitType(RANDOM),
  nbClusters_(nbClusters > 0 ? nbClusters : 1),
  dimension_(1),
  training_maxIterations_(DEFAULT_MAX_ITERATIONS),
  training_relativeDistanceThreshold_(DEFAULT_RELATIVE_VARIATION_THRESHOLD()),
  trainingCallbackFunction_(NULL)
{
    if (this->trainingSet)
        dimension_ = this->trainingSet->dimension();
    centers.resize(nbClusters*dimension_, 0.0);
}

xmm::KMeans::KMeans(KMeans const& src)
{
    this->nbClusters_ = src.nbClusters_;
    this->dimension_ = src.dimension_;
    this->centers = src.centers;
}

xmm::KMeans& xmm::KMeans::operator=(KMeans const& src)
{
    if(this != &src)
    {
        this->nbClusters_ = src.nbClusters_;
        this->dimension_ = src.dimension_;
        this->centers = src.centers;
    }
    return *this;
}

xmm::KMeans::~KMeans()
{
    centers.clear();
}

void xmm::KMeans::notify(std::string attribute)
{
    if (!trainingSet) return;
    if (attribute == "dimension") {
        dimension_ = trainingSet->dimension();
        return;
    }
    if (attribute == "destruction") {
        trainingSet = NULL;
        return;
    }
}

#pragma mark > Accessors
#pragma mark -
#pragma mark Accessors
void xmm::KMeans::set_trainingSet(TrainingSet *trainingSet)
{
    this->trainingSet = trainingSet;
}

unsigned int xmm::KMeans::get_nbClusters()
{
    return nbClusters_;
}

void xmm::KMeans::set_nbClusters(unsigned int nbClusters)
{
    nbClusters_ = nbClusters;
    centers.resize(nbClusters * dimension_, 0.0);
}

unsigned int xmm::KMeans::get_training_maxIterations() const
{
    return training_maxIterations_;
}

void xmm::KMeans::set_training_maxIterations(unsigned int maxIterations)
{
    training_maxIterations_ = maxIterations;
}

unsigned int xmm::KMeans::get_training_relativeDistanceThreshold() const
{
    return training_relativeDistanceThreshold_;
}

void xmm::KMeans::set_training_relativeDistanceThreshold(float threshold)
{
    training_relativeDistanceThreshold_ = threshold;
}

unsigned int xmm::KMeans::dimension() const
{
    return dimension_;
}

#pragma mark > Training
void xmm::KMeans::train()
{
    if (!this->trainingSet || this->trainingSet->is_empty())
    {
        if (this->trainingCallbackFunction_) {
            this->trainingCallbackFunction_(this, ProbabilisticModel::TRAINING_ERROR, this->trainingExtradata_);
        }
        return;
    }
    
    dimension_ = trainingSet->dimension();
    centers.resize(nbClusters_ * dimension_, 0.0);
    if (trainingInitType == RANDOM)
        randomizeClusters();
    else
        initClustersWithFirstPhrase();
    
    trainingNbIterations = 0;
    for (trainingNbIterations=0; trainingNbIterations<training_maxIterations_; ++trainingNbIterations) {
        std::vector<float> previous_centers = centers;
        
        updateCenters(previous_centers);
        
        float meanClusterDistance(0.0);
        float maxRelativeCenterVariation(0.0);
        for (unsigned int k=0; k<nbClusters_; ++k) {
            for (unsigned int l=0; l<nbClusters_; ++l) {
                if (k != l) {
                    meanClusterDistance += euclidian_distance(&centers[k*dimension_], &centers[l*dimension_], dimension_);
                }
            }
            maxRelativeCenterVariation = kmax(euclidian_distance(&previous_centers[k*dimension_], &centers[k*dimension_], dimension_),
                                             maxRelativeCenterVariation);
        }
        meanClusterDistance /= float(nbClusters_ * (nbClusters_ - 1));
        maxRelativeCenterVariation /= float(nbClusters_);
        maxRelativeCenterVariation /= meanClusterDistance;
        if (maxRelativeCenterVariation < training_relativeDistanceThreshold_)
            break;
        
        if (this->trainingCallbackFunction_) {
            this->trainingCallbackFunction_(this, ProbabilisticModel::TRAINING_RUN, this->trainingExtradata_);
        }
    }
    
    if (trainingCallbackFunction_) {
        trainingCallbackFunction_(this, ProbabilisticModel::TRAINING_DONE, trainingExtradata_);
    }
}

void xmm::KMeans::initClustersWithFirstPhrase()
{
    if (!this->trainingSet || this->trainingSet->is_empty())
        return;
    int step = this->trainingSet->begin()->second->length() / nbClusters_;
    
    int offset(0);
    for (int c=0; c<nbClusters_; c++) {
        for (int d=0; d<dimension_; d++) {
            centers[c*dimension_+d] = 0.0;
        }
        for (int t=0; t<step; t++) {
            for (int d=0; d<dimension_; d++) {
                centers[c*dimension_+d] += (*this->trainingSet->begin()->second)(offset+t, d) / float(step);
            }
        }
        offset += step;
    }
}


void xmm::KMeans::randomizeClusters()
{
    std::vector<float> trainingSetVariance(dimension_, 1.);
    if (trainingSet)
        std::vector<float> trainingSetVariance = trainingSet->variance();
    for (unsigned int k=0; k<nbClusters_; ++k) {
        for (unsigned int d=0; d<dimension_; ++d) {
            centers[k*dimension_ + d] = trainingSetVariance[d] * (2. * random() / float(RAND_MAX) - 1.);
        }
    }
}

void xmm::KMeans::updateCenters(std::vector<float>& previous_centers)
{
    unsigned int phraseIndex(0);
    centers.assign(nbClusters_*dimension_, 0.0);
    std::vector<unsigned int> numFramesPerCluster(nbClusters_, 0);
    for (TrainingSet::phrase_iterator it=trainingSet->begin(); it!=trainingSet->end(); ++it, ++phraseIndex) {
        for (unsigned int t=0; t<it->second->length(); ++t) {
            float min_distance;
            if (trainingSet->is_bimodal()) {
                std::vector<float> frame(dimension_);
                for (unsigned int d=0; d<dimension_; ++d) {
                    frame[d] = it->second->at(t, d);
                }
                min_distance = euclidian_distance(&frame[0],
                                                  &previous_centers[0],
                                                  dimension_);
            } else {
                min_distance = euclidian_distance(it->second->get_dataPointer(t),
                                                  &previous_centers[0],
                                                  dimension_);
            }
            int cluster_membership(0);
            for (unsigned int k=1; k<nbClusters_; ++k) {
                float distance;
                if (trainingSet->is_bimodal()) {
                    std::vector<float> frame(dimension_);
                    for (unsigned int d=0; d<dimension_; ++d) {
                        frame[d] = it->second->at(t, d);
                    }
                    distance = euclidian_distance(&frame[0],
                                                  &previous_centers[k*dimension_],
                                                  dimension_);
                } else {
                    distance = euclidian_distance(it->second->get_dataPointer(t),
                                                  &previous_centers[k*dimension_],
                                                  dimension_);
                }
                if (distance < min_distance)
                {
                    cluster_membership = k;
                    min_distance = distance;
                }
            }
            numFramesPerCluster[cluster_membership]++;
            for (unsigned int d=0; d<dimension_; ++d) {
                centers[cluster_membership * dimension_ + d] += it->second->at(t, d);
            }
        }
    }
    for (unsigned int k=0; k<nbClusters_; ++k) {
        if (numFramesPerCluster[k] > 0)
            for (unsigned int d=0; d<dimension_; ++d) {
                centers[k * dimension_ + d] /= float(numFramesPerCluster[k]);
            }
    }
}

void xmm::KMeans::set_trainingCallback(void (*callback)(void *srcModel, ProbabilisticModel::CALLBACK_FLAG state, void* extradata), void* extradata)
{
    trainingExtradata_ = extradata;
    trainingCallbackFunction_ = callback;
}

#pragma mark > Performance
void xmm::KMeans::performance_init()
{
    results_distances.resize(nbClusters_, 0.0);
}

void xmm::KMeans::performance_update(std::vector<float> const& observation)
{
    if (observation.size() != dimension_)
        throw std::runtime_error("Dimensions Don't Agree");
    results_likeliest = 0;
    float minDistance(std::numeric_limits<float>::max());
    for (unsigned int k=0; k<nbClusters_; ++k) {
        results_distances[k] = euclidian_distance(&observation[0], &centers[k*dimension_], dimension_);
        if (results_distances[k] < minDistance) {
            minDistance = results_distances[k];
            results_likeliest = k;
        }
    }
}

#pragma mark -
#pragma mark File IO
JSONNode xmm::KMeans::to_json() const
{
    JSONNode json_model(JSON_NODE);
    json_model.set_name("KMeans");
    
    json_model.push_back(JSONNode("dimension", dimension_));
    json_model.push_back(JSONNode("nbclusters", nbClusters_));
    json_model.push_back(vector2json(centers, "centers"));
    
    return json_model;
}

void xmm::KMeans::from_json(JSONNode root)
{
    try {
        if (root.type() != JSON_NODE)
            throw JSONException("Wrong type: was expecting 'JSON_NODE'", root.name());
        JSONNode::const_iterator root_it = root.begin();
        
        // Get Dimension
        root_it = root.find("dimension");
        if (root_it == root.end())
            throw JSONException("JSON Node is incomplete", root_it->name());
        if (root_it->type() != JSON_NUMBER)
            throw JSONException("Wrong type for node 'dimension': was expecting 'JSON_NUMBER'", root_it->name());
        dimension_ = static_cast<unsigned int>(root_it->as_int());
        
        // Get Number of Clusters
        root_it = root.find("nbclusters");
        if (root_it == root.end())
            throw JSONException("JSON Node is incomplete", root_it->name());
        if (root_it->type() != JSON_NUMBER)
            throw JSONException("Wrong type for node 'nbclusters': was expecting 'JSON_NUMBER'", root_it->name());
        nbClusters_ = static_cast<unsigned int>(root_it->as_int());
        
        // Get Cluster Centers
        root_it = root.find("centers");
        if (root_it == root.end())
            throw JSONException("JSON Node is incomplete", root_it->name());
        if (root_it->type() != JSON_ARRAY)
            throw JSONException("Wrong type for node 'centers': was expecting 'JSON_ARRAY'", root_it->name());
        json2vector(*root_it, centers, nbClusters_);
    } catch (JSONException &e) {
        throw JSONException(e, root.name());
    } catch (std::exception &e) {
        throw JSONException(e, root.name());
    }
}

# pragma mark > Utility
template <typename T>
T xmm::euclidian_distance(const T* vector1,
                          const T* vector2,
                          unsigned int dimension) {
    T distance(0.0);
    for (unsigned int d=0; d<dimension; d++) {
        distance += (vector1[d] - vector2[d]) * (vector1[d] - vector2[d]);
    }
    return sqrt(distance);
}


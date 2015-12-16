//
// Created by hobbang5 on 2015-12-15.
//


#include <vector>
#include <algorithm>
#include <cstdio>
#include <glm/gtc/constants.hpp>
#include <logger.h>

#include "sequence.h"

#define NUM_CIRCLE_PTS 8


using namespace glm;

bool comp(vec4 a, vec4 b);

Sequence::Sequence() {
    InitRefVector();
}

Sequence::~Sequence() {

}

void Sequence::AddRawSequence(std::vector<glm::vec3> seq) {


}

void Sequence::InitRefVector() {

    for(int i = 0; i < NUM_CIRCLE_PTS; i++)
    {
        mRefVector.push_back(vec4(cos(2 * i * pi<float>() / NUM_CIRCLE_PTS),
                                  sin(2 * i * pi<float>() / NUM_CIRCLE_PTS),
                                  0, 0));
        float z = sin(pi<float>() / 4.0f);
        mRefVector.push_back(z * vec4(cos(2 * i * pi<float>() / NUM_CIRCLE_PTS),
                                      sin(2 * i * pi<float>() / NUM_CIRCLE_PTS),
                                      1, 0));
         z = -z;
        mRefVector.push_back(z * vec4(cos(2 * i * pi<float>() / NUM_CIRCLE_PTS),
                                      sin(2 * i * pi<float>() / NUM_CIRCLE_PTS),
                                      1, 0));
    }


    mRefVector.push_back(vec4(0, 0, 1.0f, 0));
    mRefVector.push_back(vec4(0, 0, -1.0f, 0));



}


int Sequence::GetRefNum(glm::vec3 v) {

    // stay
    if (length(v) < 0.8f) {
        DBG("GetRefNum : seq %d\n", mRefVector.size());
        return mRefVector.size();
    }


    // moving
    normalize(v);
    for(int i=0; i < mRefVector.size(); i++)
        mRefVector[i].w = dot(normalize(vec3(mRefVector[i])), v);

    sort(mRefVector.begin(), mRefVector.end(), comp);
    DBG("GetRefNum sorted vec4(%3f, %3f, %3f, %3f)\n", mRefVector[0].x, mRefVector[0].y, mRefVector[0].z, mRefVector[0].w);


    return 0;
}


// compare function
bool comp(vec4 a, vec4 b) {
    return (a.w > b.w);
}
// compare object
struct comp_object {
    bool operator() (int a, int b) {
        return (a > b);
    };
} comp_obj;

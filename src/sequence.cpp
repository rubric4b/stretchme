//
// Created by hobbang5 on 2015-12-15.
//


#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <cstdio>
#include <glm/gtc/constants.hpp>
#include <logger.h>
#include <sstream>

#include "sequence.h"

#define NUM_CIRCLE_PTS 6

using namespace std;
using namespace glm;


bool comp(vec4 a, vec4 b);

void Tokenize(const string& str,
              vector<string>& tokens,
              const string& delimiters = " ")
{
    // 맨 첫 글자가 구분자인 경우 무시
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // 구분자가 아닌 첫 글자를 찾는다
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // token을 찾았으니 vector에 추가한다
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // 구분자를 뛰어넘는다.  "not_of"에 주의하라
        lastPos = str.find_first_not_of(delimiters, pos);
        // 다음 구분자가 아닌 글자를 찾는다
        pos = str.find_first_of(delimiters, lastPos);
    }
}

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
        mRefVector.push_back(vec3(cos(2.0f * (float)i * pi<float>() / (float)NUM_CIRCLE_PTS),
                                  sin(2.0f * (float)i * pi<float>() / (float)NUM_CIRCLE_PTS),
                                  0));
        float z = sin(pi<float>() / 4.0f);
        mRefVector.push_back(z * vec3(cos(2.0f * (float)i * pi<float>() / (float)NUM_CIRCLE_PTS),
                                      sin(2.0f * (float)i * pi<float>() / (float)NUM_CIRCLE_PTS),
                                      1.0f));
         z = -z;
        mRefVector.push_back(z * vec3(cos(2.0f * (float)i * pi<float>() / (float)NUM_CIRCLE_PTS),
                                      sin(2.0f * (float)i * pi<float>() / (float)NUM_CIRCLE_PTS),
                                      1.0f));
    }


    mRefVector.push_back(vec3(0, 0, 1.0f));
    mRefVector.push_back(vec3(0, 0, -1.0f));



}


int Sequence::GetRefNum(const vec3& v) {

    // stay
    if (length(v) < 1.0) {
        return -1;
    }


    // moving
    vec3 nor_v = normalize(v);
    double max = -1.0;
    int index = 0;
    for(int i=0; i < mRefVector.size(); i++) {
        double value = dot(normalize(vec3(mRefVector[i])), nor_v);
        if ( value > max ) {
            max = value;
            index = i;
        }
    }

    return index;
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

void Sequence::CreateSymbols(vector<vec3> vectors) {

    int prev_num = -1;

    for(vector<vec3>::iterator itr = vectors.begin();
        itr != vectors.end(); itr++)
    {
        int num = GetRefNum(*itr);
        if(num >= 0 && prev_num != num)
            mSymbols.push_back(num);

        prev_num = num;
    }

}

void Sequence::PrintSymbols() {
    stringstream buff_s;
    int cnt = 0;
    for(vector<int>::iterator itr = mSymbols.begin();
        itr != mSymbols.end(); itr++)
    {
//        fprintf(fp, "%d ", *itr);
        buff_s << *itr << ',';
        cnt++;
        if(cnt > 20) {
            buff_s << endl;
            cnt = 0;
        }
    }

    string buff = buff_s.str();

    vector<string> tokens;
    Tokenize(buff, tokens, "\n");
    for(vector<string>::iterator itr = tokens.begin();
        itr != tokens.end(); itr++)
    {
        DBG("Sequence: %s\n", itr.base()->c_str());

    }

}


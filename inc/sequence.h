//
// Created by hobbang5 on 2015-12-15.
//

#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__


#include <vector>
#include <glm/glm.hpp>

class Sequence {

    //methods
public:
    Sequence();
    ~Sequence();


    void AddRawSequence(std::vector<glm::vec3> seq);
    int GetRefNum(glm::vec3 v);

private:
    void InitRefVector();





    //variables
public:



private:
    std::vector<glm::vec4> mRefVector;

    int mSeqCount;


};



#endif //__SEQUENCE_H__

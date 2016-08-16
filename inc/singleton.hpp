//
// Created by hobbang5 on 2016-03-24.
//

#ifndef STRETCHME_SINGLETON_H
#define STRETCHME_SINGLETON_H

template <typename T>
class Singleton
{
public:
    static T * get_InstancePtr() {
        if(m_instance == NULL ) {
            m_instance = new T;
        }
        return m_instance;
    };

    static T & Inst() {
        if(m_instance == NULL) {
            m_instance = new T;
        }
        return *m_instance;
    };

    static void release_Instance()
    {
        if(m_instance) {
            delete m_instance;
            m_instance = NULL;
        }
    };

private:
    static T *m_instance;

protected:
    Singleton() { };
    virtual ~Singleton() { };

};

template<typename T>
T* Singleton<T>::m_instance = 0;


#endif //STRETCHME_SINGLETON_H

/*Copyright (c) 2013, Edgar Solomonik, all rights reserved.*/

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include "../shared/util.h"
#include "../../include/ctf.hpp"

//template<typename dtype>
//tCTF_Idx_Tensor<dtype> get_intermediate(tCTF_Idx_Tensor<dtype>& A, 
//                                        tCTF_Idx_Tensor<dtype>& B){
//  int * len_C, * sym_C;
//  char * idx_C;
//  int ndim_C, i, j, idx;
//  
//  ndim_C = 0;
//  for (i=0; i<A.parent->ndim; i++){
//    ndim_C++;
//    for (j=0; j<B.parent->ndim; j++){
//      if (A.idx_map[i] == B.idx_map[j]){
//        ndim_C--;
//        break;
//      }
//    }
//  }
//  for (j=0; j<B.parent->ndim; j++){
//    ndim_C++;
//    for (i=0; i<A.parent->ndim; i++){
//      if (A.idx_map[i] == B.idx_map[j]){
//        ndim_C--;
//        break;
//      }
//    }
//  }
//
//  idx_C = (char*)CTF_alloc(sizeof(char)*ndim_C);
//  sym_C = (int*)CTF_alloc(sizeof(int)*ndim_C);
//  len_C = (int*)CTF_alloc(sizeof(int)*ndim_C);
//  idx = 0;
//  for (i=0; i<A.parent->ndim; i++){
//    for (j=0; j<B.parent->ndim; j++){
//      if (A.idx_map[i] == B.idx_map[j]){
//        break;
//      }
//    }
//    if (j == B.parent->ndim){
//      idx_C[idx] = A.idx_map[i];
//      len_C[idx] = A.parent->len[i];
//      if (idx >= 1 && i >= 1 && idx_C[idx-1] == A.idx_map[i-1] && A.parent->sym[i-1] != NS){
//        sym_C[idx-1] = A.parent->sym[i-1];
//      }
//      sym_C[idx] = NS;
//      idx++;
//    }
//  }
//  for (j=0; j<B.parent->ndim; j++){
//    for (i=0; i<A.parent->ndim; i++){
//      if (A.idx_map[i] == B.idx_map[j]){
//        break;
//      }
//    }
//    if (i == A.parent->ndim){
//      idx_C[idx] = B.idx_map[j];
//      len_C[idx] = B.parent->len[j];
//      if (idx >= 1 && j >= 1 && idx_C[idx-1] == B.idx_map[j-1] && B.parent->sym[j-1] != NS){
//        sym_C[idx-1] = B.parent->sym[j-1];
//      }
//      sym_C[idx] = NS;
//      idx++;
//    }
//  }
//
//  tCTF_Tensor<dtype> * tsr_C = new tCTF_Tensor<dtype>(ndim_C, len_C, sym_C, *(A.parent->world));
//  tCTF_Idx_Tensor<dtype> out(tsr_C, idx_C);
//  out.is_intm = 1;
//  CTF_free(sym_C);
//  CTF_free(len_C);
//  return out;
//}

template<typename dtype>
tCTF_Idx_Tensor<dtype>::tCTF_Idx_Tensor(tCTF_Tensor<dtype> *  parent_, 
                                        const char *          idx_map_, 
                                        int                   copy){
  if (copy){
    parent = new tCTF_Tensor<dtype>(*parent,1);
    idx_map = (char*)CTF_alloc(parent->ndim*sizeof(char));
  } else {
    idx_map = (char*)CTF_alloc(parent_->ndim*sizeof(char));
    parent        = parent_;
  }
  memcpy(idx_map, idx_map_, parent->ndim*sizeof(char));
  is_intm       = 0;
  this->scale    = 1.0;
}

template<typename dtype>
tCTF_Idx_Tensor<dtype>::tCTF_Idx_Tensor(
    tCTF_Idx_Tensor<dtype> const &  other,
    int                       copy) {
  if (copy){
    parent = new tCTF_Tensor<dtype>(*other.parent,1);
    idx_map = (char*)CTF_alloc(other.parent->ndim*sizeof(char));
  } else {
    idx_map = (char*)CTF_alloc(other.parent->ndim*sizeof(char));
    parent        = other.parent;
  }
  memcpy(idx_map, other.idx_map, parent->ndim*sizeof(char));
  is_intm       = other.is_intm;
  this->scale    = other.scale;
}

template<typename dtype>
tCTF_Idx_Tensor<dtype>::tCTF_Idx_Tensor(){
  parent        = NULL;
  idx_map       = NULL;
  is_intm       = 0;
  this->scale    = 1.0;
}

template<typename dtype>
tCTF_Idx_Tensor<dtype>::~tCTF_Idx_Tensor(){
  CTF_free(idx_map);
}


template<typename dtype>
void tCTF_Idx_Tensor<dtype>::operator=(tCTF_Term<dtype> B){
  this->scale = 0.0;
  B.execute(*this);
}

template<typename dtype>
void tCTF_Idx_Tensor<dtype>::operator+=(tCTF_Term<dtype> B){
  this->scale = 1.0;
  B.execute(*this);
}

template<typename dtype>
void tCTF_Idx_Tensor<dtype>::operator-=(tCTF_Term<dtype> B){
  this->scale = 1.0;
  B.scale *= -1.0;
  B.execute(*this);
}

template<typename dtype>
void tCTF_Idx_Tensor<dtype>::operator*=(tCTF_Term<dtype> B){
  tCTF_Contract_Term<dtype> ctrm = (*this)*B;
  *this = ctrm;
}

template<typename dtype>
void tCTF_Idx_Tensor<dtype>::execute(tCTF_Idx_Tensor<dtype> output){
  output.parent->sum(this->scale, *this->parent, idx_map,
                     output.scale, output.idx_map);
}

template<typename dtype>
tCTF_Idx_Tensor<dtype> tCTF_Idx_Tensor<dtype>::execute(){
  return *this;
}

/*template<typename dtype>
void tCTF_Idx_Tensor<dtype>::operator=(dtype B){
  *this=(tCTF_Scalar<dtype>(B,*(this->parent->world))[""]);
}
template<typename dtype>
void tCTF_Idx_Tensor<dtype>::operator+=(dtype B){
  *this+=(tCTF_Scalar<dtype>(B,*(this->parent->world))[""]);
}
template<typename dtype>
void tCTF_Idx_Tensor<dtype>::operator-=(dtype B){
  *this-=(tCTF_Scalar<dtype>(B,*(this->parent->world))[""]);
}
template<typename dtype>
void tCTF_Idx_Tensor<dtype>::operator*=(dtype B){
  *this*=(tCTF_Scalar<dtype>(B,*(this->parent->world))[""]);
}*/

/*
template<typename dtype>
tCTF_Idx_Tensor<dtype> tCTF_Idx_Tensor<dtype>::operator+(tCTF_Idx_Tensor<dtype> tsr){
  if (has_contract || has_sum){
    *NBR = (*NBR)-tsr;
    return *this;
  }
  NBR = &tsr;
  has_sum = 1;
  return *this;
}

template<typename dtype>
tCTF_Idx_Tensor<dtype> tCTF_Idx_Tensor<dtype>::operator-(tCTF_Idx_Tensor<dtype> tsr){
  if (has_contract || has_sum){
    *NBR = (*NBR)-tsr;
    return *this;
  }
  NBR = &tsr;
  has_sum = 1;
  if (tsr.has_scale) tsr.scale = -1.0*tsr.scale;
  else {
    tsr.has_scale = 1;
    tsr.scale = -1.0;
  }
  return *this;
}

template<typename dtype>
tCTF_Idx_Tensor<dtype> tCTF_Idx_Tensor<dtype>::operator*(double  scl){
  if (has_contract){
    *NBR =(*NBR)*scl;
    return *this;
  }
  if (has_scale){
    scale *= scl;
  } else {
    has_scale = 1;
    scale = scl;
  }
  return *this;
}*/

/*template<typename dtype>
tCTF_Idx_Tensor<dtype>::operator dtype(){
  tCTF_Scalar<dtype> sc(*parent->world);
  tCTF_Idx_Tensor<dtype> * isc = new tCTF_Idx_Tensor<dtype>(&sc,""); 
  run(isc, 0.0);
//  delete isc;
  return sc.get_val();
}*/
/*
template<typename dtype>
void tCTF_Idx_Tensor<dtype>::run(tCTF_Idx_Tensor<dtype>* output, dtype  beta){
  dtype  alpha;
  if (has_scale) alpha = scale;
  else alpha = 1.0;
  if (has_contract){
    if (NBR->has_scale){
      alpha *= NBR->scale;
    }
    if (NBR->has_contract || NBR->has_sum){
      tCTF_Idx_Tensor itsr = get_intermediate(*this,*NBR);
      itsr.has_sum = NBR->has_sum;
      itsr.has_contract = NBR->has_contract;
      itsr.NBR = NBR->NBR;
      printf("interm tsr has_Contract = %d, NBR = %p, NBR.has_scale = %d\n", itsr.has_contract, itsr.NBR,
      itsr.NBR->has_scale);
      
      itsr.parent->contract(alpha, *(this->parent), this->idx_map,
                                    *(NBR->parent),  NBR->idx_map,
                             0.0,                    itsr.idx_map);
      itsr.run(output, beta);
    } else {
      output->parent->contract(alpha, *(this->parent), this->idx_map,
                                      *(NBR->parent),  NBR->idx_map,
                               beta,                   output->idx_map);
    }
  } else {
    if (has_sum){
      tCTF_Tensor<dtype> tcpy(*(this->parent),1);
      tCTF_Idx_Tensor itsr(&tcpy, idx_map);
      NBR->run(&itsr, alpha);
      output->parent->sum(1.0, tcpy, idx_map, beta, output->idx_map);
//      delete itsr;
//      delete tcpy;
    } else {
      output->parent->sum(alpha, *(this->parent), idx_map, beta, output->idx_map);
    }
  }  
//  if (!is_perm)
//    delete this;
}*/




template class tCTF_Idx_Tensor<double>;
template class tCTF_Idx_Tensor< std::complex<double> >;

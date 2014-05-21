#ifndef __CTF_TENSOR_H__
#define __CTF_TENSOR_H__

#include "ctf_world.h"

/**
 * \brief index-value pair used for tensor data input
 */
template<typename dtype=double>
class CTF_pair {
  long_int k;
  dtype d;
  CTF_pair() {}
  CTF_pair(long_int k, dtype d) : k(k), d(d) {}
  bool operator< (const CTF_pair<dtype>& other) const{
    return k < other.k;
  }
  bool operator==(const CTF_pair<dtype>& other) const{
    return (k == other.k && d == other.d);
  }
  bool operator!=(const CTF_pair<dtype>& other) const{
    return !(*this == other);
  }
};

template<typename dtype>
inline bool comp_CTF_pair(CTF_pair<dtype> i,
                          CTF_pair<dtype> j) {
  return (i.k<j.k);
}

/**
 * \defgroup CTF CTF: C++ Tensor interface
 * @{
 */

/**
 * \brief an instance of a tensor within a CTF world
 */
template <typename dtype=double>
class CTF_Tensor {
  public:
    int tid, ndim;
    int * sym, * len;
    char * idx_map;
    char const * name;
    CTF_World * world;
    CTF_Semiring semiring;

  public:
    /**
     * \breif default constructor
     */
    CTF_Tensor();

    /**
     * \brief copies a tensor (setting data to zero or copying A)
     * \param[in] A tensor to copy
     * \param[in] copy whether to copy the data of A into the new tensor
     */
    CTF_Tensor(CTF_Tensor const &   A,
                bool                  copy = true);

    /**
     * \brief copies a tensor filled with zeros
     * \param[in] ndim_ number of dimensions of tensor
     * \param[in] len_ edge lengths of tensor
     * \param[in] sym_ symmetries of tensor (e.g. symmetric matrix -> sym={SY, NS})
     * \param[in] world_ a world for the tensor to live in
     * \param[in] name_ an optionary name for the tensor
     * \param[in] profile_ set to 1 to profile contractions involving this tensor
     */
    CTF_Tensor(int            ndim_,
                int const *    len_,
                int const *    sym_,
                CTF_World &    world_,
#if DEBUG < 3
                char const *   name_ = NULL,
                int            profile_ = 0
#else
                char const *   name_ = "X",
                int            profile_ = 1
#endif
                 );
    /**
     * \brief copies a tensor filled with zeros
     * \param[in] ndim_ number of dimensions of tensor
     * \param[in] len_ edge lengths of tensor
     * \param[in] sym_ symmetries of tensor (e.g. symmetric matrix -> sym={SY, NS})
     * \param[in] world_ a world for the tensor to live in
     * \param[in] semiring_ defines the tensor arithmetic for this tensor
     * \param[in] name_ an optionary name for the tensor
     * \param[in] profile_ set to 1 to profile contractions involving this tensor
     */
    CTF_Tensor(int            ndim_,
                int const *    len_,
                int const *    sym_,
                CTF_World &    world_,
                CTF_Semiring   semiring_,
                char const *   name_ = NULL,
                int            profile_ = 0);
    
    /**
     * \brief creates a zeroed out copy (data not copied) of a tensor in a different world
     * \param[in] A tensor whose characteristics to copy
     * \param[in] world_ a world for the tensor we are creating to live in, can be different from A
     */
    CTF_Tensor(CTF_Tensor const & A,
                CTF_World        & world_);

    /**
     * \brief gives the values associated with any set of indices
     * The sparse data is defined in coordinate format. The tensor index (i,j,k,l) of a tensor with edge lengths
     * {m,n,p,q} is associated with the global index g via the formula g=i+j*m+k*m*n+l*m*n*p. The row index is first
     * and the column index is second for matrices, which means they are column major. 
     * \param[in] npair number of values to fetch
     * \param[in] global_idx index within global tensor of each value to fetch
     * \param[in,out] data a prealloced pointer to the data with the specified indices
     */
    void read(long_int          npair, 
              long_int const *  global_idx, 
              dtype *           data) const;
    
    /**
     * \brief gives the values associated with any set of indices
     * \param[in] npair number of values to fetch
     * \param[in,out] pairs a prealloced pointer to key-value pairs
     */
    void read(long_int          npair,
              CTF_pair<dtype> * pairs) const;
    
    /**
     * \brief sparse read: A[global_idx[i]] = alpha*A[global_idx[i]]+beta*data[i]
     * \param[in] npair number of values to read into tensor
     * \param[in] alpha scaling factor on read data
     * \param[in] beta scaling factor on value in initial values vector
     * \param[in] global_idx global index within tensor of value to add
     * \param[in] data values to add to the tensor
     */
    void read(long_int         npair, 
              dtype            alpha, 
              dtype            beta,
              long_int const * global_idx,
              dtype *          data) const;

    /**
     * \brief sparse read: pairs[i].d = alpha*A[pairs[i].k]+beta*pairs[i].d
     * \param[in] npair number of values to read into tensor
     * \param[in] alpha scaling factor on read data
     * \param[in] beta scaling factor on value in initial pairs vector
     * \param[in] pairs key-value pairs to add to the tensor
     */
    void read(long_int          npair,
              dtype             alpha,
              dtype             beta,
              CTF_pair<dtype> * pairs) const;
   

    /**
     * \brief writes in values associated with any set of indices
     * The sparse data is defined in coordinate format. The tensor index (i,j,k,l) of a tensor with edge lengths
     * {m,n,p,q} is associated with the global index g via the formula g=i+j*m+k*m*n+l*m*n*p. The row index is first
     * and the column index is second for matrices, which means they are column major. 
     * \param[in] npair number of values to write into tensor
     * \param[in] global_idx global index within tensor of value to write
     * \param[in] data values to  write to the indices
     */
    void write(long_int         npair, 
               long_int const * global_idx, 
               dtype const    * data);

    /**
     * \brief writes in values associated with any set of indices
     * \param[in] npair number of values to write into tensor
     * \param[in] pairs key-value pairs to write to the tensor
     */
    void write(long_int                 npair,
               CTF_pair<dtype> const *  pairs);
    
    /**
     * \brief sparse add: A[global_idx[i]] = beta*A[global_idx[i]]+alpha*data[i]
     * \param[in] npair number of values to write into tensor
     * \param[in] alpha scaling factor on value to add
     * \param[in] beta scaling factor on original data
     * \param[in] global_idx global index within tensor of value to add
     * \param[in] data values to add to the tensor
     */
    void write(long_int         npair, 
               dtype            alpha, 
               dtype            beta,
               long_int const * global_idx,
               dtype const *    data);

    /**
     * \brief sparse add: A[pairs[i].k] = alpha*A[pairs[i].k]+beta*pairs[i].d
     * \param[in] npair number of values to write into tensor
     * \param[in] alpha scaling factor on value to add
     * \param[in] beta scaling factor on original data
     * \param[in] pairs key-value pairs to add to the tensor
     */
    void write(long_int                npair,
               dtype                   alpha,
               dtype                   beta,
               CTF_pair<dtype> const * pairs);
   
    /**
     * \brief contracts C[idx_C] = beta*C[idx_C] + alpha*A[idx_A]*B[idx_B]
     *        if fseq defined computes fseq(alpha,A[idx_A],B[idx_B],beta*C[idx_C])
     * \param[in] alpha A*B scaling factor
     * \param[in] A first operand tensor
     * \param[in] idx_A indices of A in contraction, e.g. "ik" -> A_{ik}
     * \param[in] B second operand tensor
     * \param[in] idx_B indices of B in contraction, e.g. "kj" -> B_{kj}
     * \param[in] beta C scaling factor
     * \param[in] idx_C indices of C (this tensor),  e.g. "ij" -> C_{ij}
     * \param[in] fseq sequential operation to execute, default is multiply-add
     */
    void contract(dtype                    alpha, 
                  const CTF_Tensor&        A, 
                  char const *             idx_A,
                  const CTF_Tensor&        B, 
                  char const *             idx_B,
                  dtype                    beta,
                  char const *             idx_C,
                  CTF_Fbilinear<dtype>     fseq = CTF_Fbilinear<dtype>());

    /**
     * \brief estimate the cost of a contraction C[idx_C] = A[idx_A]*B[idx_B]
     * \param[in] A first operand tensor
     * \param[in] idx_A indices of A in contraction, e.g. "ik" -> A_{ik}
     * \param[in] B second operand tensor
     * \param[in] idx_B indices of B in contraction, e.g. "kj" -> B_{kj}
     * \param[in] idx_C indices of C (this tensor),  e.g. "ij" -> C_{ij}
     * \return cost as a int64_t type, currently a rought estimate of flops/processor
     */
    int64_t estimate_cost(const CTF_Tensor & A,
                          char const *        idx_A,
                          const CTF_Tensor & B,
                          char const *        idx_B,
                          char const *        idx_C);
    
    /**
     * \brief estimate the cost of a sum B[idx_B] = A[idx_A]
     * \param[in] A first operand tensor
     * \param[in] idx_A indices of A in contraction, e.g. "ik" -> A_{ik}
     * \param[in] idx_B indices of B in contraction, e.g. "kj" -> B_{kj}
     * \return cost as a int64_t type, currently a rought estimate of flops/processor
     */
    int64_t estimate_cost(const CTF_Tensor & A,
                          char const *        idx_A,
                          char const *        idx_B);

    
    /**
     * \brief sums B[idx_B] = beta*B[idx_B] + alpha*A[idx_A]
     *        if fseq defined computes fseq(alpha,A[idx_A],beta*B[idx_B])
     * \param[in] alpha A scaling factor
     * \param[in] A first operand tensor
     * \param[in] idx_A indices of A in sum, e.g. "ij" -> A_{ij}
     * \param[in] beta B scaling factor
     * \param[in] idx_B indices of B (this tensor), e.g. "ij" -> B_{ij}
     * \param[in] fseq sequential operation to execute, default is multiply-add
     */
    void sum(dtype                   alpha, 
             const CTF_Tensor&      A, 
             char const *            idx_A,
             dtype                   beta,
             char const *            idx_B,
             CTF_fsum<dtype>        fseq = CTF_fsum<dtype>());
    
    /**
     * \brief scales A[idx_A] = alpha*A[idx_A]
     *        if fseq defined computes fseq(alpha,A[idx_A])
     * \param[in] alpha A scaling factor
     * \param[in] idx_A indices of A (this tensor), e.g. "ij" -> A_{ij}
     * \param[in] fseq sequential operation to execute, default is multiply-add
     */
    void scale(dtype                   alpha, 
               char const *            idx_A,
               CTF_fscl<dtype>        fseq = CTF_fscl<dtype>());

    /**
     * \brief cuts out a slice (block) of this tensor A[offsets,ends)
     * \param[in] offsets bottom left corner of block
     * \param[in] ends top right corner of block
     * \return new tensor corresponding to requested slice
     */
    CTF_Tensor slice(int const * offsets,
                      int const * ends) const;
    
    CTF_Tensor slice(long_int corner_off,
                      long_int corner_end) const;
    
    /**
     * \brief cuts out a slice (block) of this tensor A[offsets,ends)
     * \param[in] offsets bottom left corner of block
     * \param[in] ends top right corner of block
     * \return new tensor corresponding to requested slice which lives on
     *          oworld
     */
    CTF_Tensor slice(int const *         offsets,
                      int const *         ends,
                      CTF_World<dtype> * oworld) const;

    CTF_Tensor slice(long_int            corner_off,
                      long_int            corner_end,
                      CTF_World<dtype> * oworld) const;
    
    
    /**
     * \brief cuts out a slice (block) of this tensor = B
     *   B[offsets,ends)=beta*B[offsets,ends) + alpha*A[offsets_A,ends_A)
     * \param[in] offsets bottom left corner of block
     * \param[in] ends top right corner of block
     * \param[in] alpha scaling factor of this tensor
     * \param[in] offsets bottom left corner of block of A
     * \param[in] ends top right corner of block of A
     * \param[in] alpha scaling factor of tensor A
     */
    void slice(int const *    offsets,
               int const *    ends,
               dtype          beta,
               CTF_Tensor const & A,
               int const *    offsets_A,
               int const *    ends_A,
               dtype          alpha) const;
    
    void slice(long_int       corner_off,
               long_int       corner_end,
               dtype          beta,
               CTF_Tensor const & A,
               long_int       corner_off_A,
               long_int       corner_end_A,
               dtype          alpha) const;

    /**
     * \brief Apply permutation to matrix, potentially extracting a slice
     *              B[i,j,...] 
     *                = beta*B[...] + alpha*A[perms_A[0][i],perms_A[1][j],...]
     *
     * \param[in] beta scaling factor for values of tensor B (this)
     * \param[in] A specification of operand tensor A must live on 
                    the same CTF_World or a subset of the CTF_World on which B lives
     * \param[in] perms_A specifies permutations for tensor A, e.g. A[perms_A[0][i],perms_A[1][j]]
     *                    if a subarray NULL, no permutation applied to this index,
     *                    if an entry is -1, the corresponding entries of the tensor are skipped 
                            (A must then be smaller than B)
     * \param[in] alpha scaling factor for A tensor
     */
    void permute(dtype          beta,
                 CTF_Tensor &  A,
                 int * const *  perms_A,
                 dtype          alpha);

    /**
     * \brief Apply permutation to matrix, potentially extracting a slice
     *              B[perms_B[0][i],perms_B[0][j],...] 
     *                = beta*B[...] + alpha*A[i,j,...]
     *
     * \param[in] perms_B specifies permutations for tensor B, e.g. B[perms_B[0][i],perms_B[1][j]]
     *                    if a subarray NULL, no permutation applied to this index,
     *                    if an entry is -1, the corresponding entries of the tensor are skipped 
     *                       (A must then be smaller than B)
     * \param[in] beta scaling factor for values of tensor B (this)
     * \param[in] A specification of operand tensor A must live on 
                    the same CTF_World or a superset of the CTF_World on which B lives
     * \param[in] alpha scaling factor for A tensor
     */
    void permute(int * const *  perms_B,
                 dtype          beta,
                 CTF_Tensor &  A,
                 dtype          alpha);
    
   /**
     * \brief accumulates this tensor to a tensor object defined on a different world
     * \param[in] tsr a tensor object of the same characteristic as this tensor, 
     *             but on a different CTF_world/MPI_comm
     * \param[in] alpha scaling factor for this tensor (default 1.0)
     * \param[in] beta scaling factor for tensor tsr (default 1.0)
     */
    void add_to_subworld(CTF_Tensor<dtype> * tsr,
                         dtype alpha,
                         dtype beta) const;
    void add_to_subworld(CTF_Tensor<dtype> * tsr) const;
    
   /**
     * \brief accumulates this tensor from a tensor object defined on a different world
     * \param[in] tsr a tensor object of the same characteristic as this tensor, 
     *             but on a different CTF_world/MPI_comm
     * \param[in] alpha scaling factor for tensor tsr (default 1.0)
     * \param[in] beta scaling factor for this tensor (default 1.0)
     */
    void add_from_subworld(CTF_Tensor<dtype> * tsr,
                           dtype alpha,
                           dtype beta) const;
    void add_from_subworld(CTF_Tensor<dtype> * tsr) const;
    

    /**
     * \brief aligns data mapping with tensor A
     * \param[in] A align with this tensor
     */
    void align(CTF_Tensor const & A);

    /**
     * \brief performs a reduction on the tensor
     * \param[in] op reduction operation (see top of this cyclopstf.hpp for choices)
     */    
    dtype reduce(CTF_OP op);
    
    /**
     * \brief computes the entrywise 1-norm of the tensor
     */    
    dtype norm1(){ return reduce(CTF_OP_NORM1); };

    /**
     * \brief computes the frobenius norm of the tensor
     */    
    dtype norm2(){ return reduce(CTF_OP_NORM2); };

    /**
     * \brief finds the max absolute value element of the tensor
     */    
    dtype norm_infty(){ return reduce(CTF_OP_MAXABS); };

    /**
     * \brief gives the raw current local data with padding included
     * \param[out] size of local data chunk
     * \return pointer to local data
     */
    dtype * get_raw_data(long_int * size);

    /**
     * \brief gives a read-only copy of the raw current local data with padding included
     * \param[out] size of local data chunk
     * \return pointer to read-only copy of local data
     */
    const dtype * raw_data(long_int * size) const;

    /**
     * \brief gives the global indices and values associated with the local data
     * \param[out] npair number of local values
     * \param[out] global_idx index within global tensor of each data value
     * \param[out] data pointer to local values in the order of the indices
     */
    void read_local(long_int *   npair, 
                    long_int **  global_idx, 
                    dtype **     data) const;

    /**
     * \brief gives the global indices and values associated with the local data
     * \param[out] npair number of local values
     * \param[out] pairs pointer to local key-value pairs
     */
    void read_local(long_int *         npair,
                    CTF_pair<dtype> ** pairs) const;

    /**
     * \brief collects the entire tensor data on each process (not memory scalable)
     * \param[out] npair number of values in the tensor
     * \param[out] data pointer to the data of the entire tensor
     */
    void read_all(long_int * npair, 
                  dtype **   data) const;
    
    /**
     * \brief collects the entire tensor data on each process (not memory scalable)
     * \param[in,out] preallocated data pointer to the data of the entire tensor
     */
    long_int read_all(dtype * data) const;

    /**
     * \brief obtains a small number of the biggest elements of the 
     *        tensor in sorted order (e.g. eigenvalues)
     * \param[in] n number of elements to collect
     * \param[in] data output data (should be preallocated to size at least n)
     *
     * WARNING: currently functional only for dtype=double
     */
    void get_max_abs(int        n,
                     dtype *    data);

    /**
     * \brief turns on profiling for tensor
     */
    void profile_on();
    
    /**
     * \brief turns off profiling for tensor
     */
    void profile_off();

    /**
     * \brief sets tensor name
     * \param[in] name new for tensor
     */
    void set_name(char const * name);

    /**
     * \brief sets all values in the tensor to val
     */
    CTF_Tensor& operator=(dtype val);
    
    /**
     * \brief sets the tensor
     */
    void operator=(CTF_Tensor<dtype> A);
    
    /**
     * \brief associated an index map with the tensor for future operation
     * \param[in] idx_map_ index assignment for this tensor
     */
    CTF_Idx_Tensor<dtype> operator[](char const * idx_map_);
    
    /**
     * \brief gives handle to sparse index subset of tensors
     * \param[in] indices, vector of indices to sparse tensor
     */
    CTF_Sparse_Tensor<dtype> operator[](std::vector<long_int> indices);
    
    /**
     * \brief prints tensor data to file using process 0
     * \param[in] fp file to print to e.g. stdout
     * \param[in] cutoff do not print values of absolute value smaller than this
     */
    void print(FILE * fp = stdout, double cutoff = -1.0) const;

    /**
     * \brief prints two sets of tensor data side-by-side to file using process 0
     * \param[in] fp file to print to e.g. stdout
     * \param[in] A tensor to compare against
     * \param[in] cutoff do not print values of absolute value smaller than this
     */
    void compare(const CTF_Tensor<dtype>& A, FILE * fp = stdout, double cutoff = -1.0) const;

    /**
     * \brief frees CTF tensor
     */
    ~CTF_Tensor();
};



/**
 * \brief Matrix class which encapsulates a 2D tensor 
 */
template<typename dtype=double> 
class CTF_Matrix : public CTF_Tensor<dtype> {
  public:
    int nrow, ncol, sym;

    /**
     * \brief constructor for a matrix
     * \param[in] nrow number of matrix rows
     * \param[in] ncol number of matrix columns
     * \param[in] sym symmetry of matrix
     * \param[in] world CTF world where the tensor will live
     * \param[in] name_ an optionary name for the tensor
     * \param[in] profile_ set to 1 to profile contractions involving this tensor
     */ 
    CTF_Matrix(int                 nrow_, 
                int                 ncol_, 
                int                 sym_,
                CTF_World<dtype> & world,
                char const *        name_ = NULL,
                int                 profile_ = 0);

};


/**
 * \brief Vector class which encapsulates a 1D tensor 
 */
template<typename dtype=double> 
class CTF_Vector : public CTF_Tensor<dtype> {
  public:
    int len;

    /**
     * \brief constructor for a vector
     * \param[in] len_ dimension of vector
     * \param[in] world CTF world where the tensor will live
     * \param[in] name_ an optionary name for the tensor
     * \param[in] profile_ set to 1 to profile contractions involving this tensor
     */ 
    CTF_Vector(int                 len_,
                CTF_World<dtype> & world,
                char const *        name_ = NULL,
                int                 profile_ = 0);
};


/**
 * \brief Scalar class which encapsulates a 0D tensor 
 */
template<typename dtype=double> 
class CTF_Scalar : public CTF_Tensor<dtype> {
  public:
    /**
     * \brief constructor for a scalar with predefined value
     * \param[in] val scalar value
     * \param[in] world CTF world where the tensor will live
     */ 
    CTF_Scalar(dtype       val,
                CTF_World & world);

    /**
     * \brief returns scalar value
     */
    dtype get_val();
    
    /**
     * \brief sets scalar value
     */
    void set_val(dtype val);

    /**
     * \brief casts into a dtype value
     */
    operator dtype() { return get_val(); }


};

/**
 * \brief a sparse subset of a tensor 
 */
template<typename dtype=double>
class CTF_Sparse_Tensor {
  public:
    CTF_Tensor<dtype> * parent;
    std::vector<long_int> indices;
    dtype scale;

    /** 
      * \brief base constructor 
      */
    CTF_Sparse_Tensor();
    
    /**
     * \brief initialize a tensor which corresponds to a set of indices 
     * \param[in] indices a vector of global indices to tensor values
     * \param[in] parent dense distributed tensor to which this sparse tensor belongs to
     */
    CTF_Sparse_Tensor(std::vector<long_int> indices,
                       CTF_Tensor<dtype> * parent);

    /**
     * \brief initialize a tensor which corresponds to a set of indices 
     * \param[in] number of values this sparse tensor will have locally
     * \param[in] indices an array of global indices to tensor values
     * \param[in] parent dense distributed tensor to which this sparse tensor belongs to
     */
    CTF_Sparse_Tensor(long_int              n,
                       long_int *            indices,
                       CTF_Tensor<dtype> * parent);

    /**
     * \brief set the sparse set of indices on the parent tensor to values
     *        forall(j) i = indices[j]; parent[i] = beta*parent[i] + alpha*values[j];
     * \param[in] alpha scaling factor on values array 
     * \param[in] values data, should be of same size as the number of indices (n)
     * \param[in] beta scaling factor to apply to previously existing data
     */
    void write(dtype              alpha, 
               dtype *            values,
               dtype              beta); 

    // C++ overload special-cases of above method
    void operator=(std::vector<dtype> values); 
    void operator+=(std::vector<dtype> values); 
    void operator-=(std::vector<dtype> values); 
    void operator=(dtype * values); 
    void operator+=(dtype * values); 
    void operator-=(dtype * values); 

    /**
     * \brief read the sparse set of indices on the parent tensor to values
     *        forall(j) i = indices[j]; values[j] = alpha*parent[i] + beta*values[j];
     * \param[in] alpha scaling factor on parent array 
     * \param[in] values data, should be preallocated to the same size as the number of indices (n)
     * \param[in] beta scaling factor to apply to previously existing data in values
     */
    void read(dtype              alpha, 
              dtype *            values,
              dtype              beta); 

    // C++ overload special-cases of above method
    operator std::vector<dtype>();
    operator dtype*();
};



/**
 * @}
 */

#endif

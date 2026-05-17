#pragma once

// One-line swap point between Intel MKL Pardiso (when available) and Eigen's
// native solvers. CMake defines S3_HAVE_MKL when MKL is detected; otherwise
// we fall back to header-only Eigen — slower (~2-5×) but no proprietary dep,
// and the only option on Apple Silicon.

#include <Eigen/Sparse>

#ifdef S3_HAVE_MKL
#  include <Eigen/PardisoSupport>
using S3SparseLDLT = Eigen::PardisoLDLT<Eigen::SparseMatrix<double>>;
using S3SparseLU   = Eigen::PardisoLU<Eigen::SparseMatrix<double>>;
#else
#  include <Eigen/SparseCholesky>
#  include <Eigen/SparseLU>
using S3SparseLDLT = Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>>;
using S3SparseLU   = Eigen::SparseLU<Eigen::SparseMatrix<double>>;
#endif

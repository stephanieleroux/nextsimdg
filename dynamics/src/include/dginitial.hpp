/*----------------------------   initial.h     ---------------------------*/
/*      $Id:$                 */
#ifndef __initial_H
#define __initial_H
/*----------------------------   initial.h     ---------------------------*/

#include "dgvector.hpp"
#include "mesh.hpp"

namespace Nextsim {

class InitialBase {
public:
    virtual double operator()(double x, double y) const = 0;
};

class SmoothInitial : virtual public InitialBase {
public:
    virtual double operator()(double x, double y) const
    {
        double r2 = ((x - 0.7) * (x - 0.7) + (y - 0.7) * (y - 0.7));
        return exp(-200.0 * r2);
    }
};

class PyramidInitial : virtual public InitialBase {
public:
    virtual double operator()(double x, double y) const
    {
        double r2 = ((x - 0.3) * (x - 0.3) + (y - 0.6) * (y - 0.6));
        return std::max(0.0, 1.0 - 10.0 * sqrt(r2));
    }
};

class BoxInitial : virtual public InitialBase {
public:
    virtual double operator()(double x, double y) const
    {
        double r2 = ((x - 0.65) * (x - 0.65) + (y - 0.3) * (y - 0.3));
        if (sqrt(r2) < 0.1)
            return 1.0;
        return 0.0;
    }
};

class MixedInitial : virtual public InitialBase {
    virtual double operator()(double x, double y) const
    {
        //    return BoxInitial()(x, y);
        return SmoothInitial()(x, y) + BoxInitial()(x, y) + PyramidInitial()(x, y);
    }
};

//////////////////////////////////////////////////

//! Functions to project an analytical solution into the DG spaces

template <int DGdegree>
void L2ProjectInitial(const Mesh& mesh,
    CellVector<DGdegree>& phi,
    const InitialBase& initial);

template <>
void L2ProjectInitial(const Mesh& mesh,
    CellVector<0>& phi,
    const InitialBase& initial)
{
    phi.setZero();

#pragma omp parallel for
    for (size_t iy = 0; iy < mesh.ny; ++iy) {
        size_t ic = mesh.nx * iy;
        for (size_t ix = 0; ix < mesh.nx; ++ix, ++ic) {
            Vertex xm = mesh.midpoint(ix, iy);
            phi(ic, 0) = initial(xm[0], xm[1]);
        }
    }
}

template <>
void L2ProjectInitial(const Mesh& mesh,
    CellVector<1>& phi,
    const InitialBase& initial)
{
    phi.setZero();

    std::array<double, 3> imass = { 1.0, 12.0, 12.0 }; // without 'h'

    // Gauss quadrature on [-1/2, 1/2]
    std::array<double, 2> g2 = { -1.0 / sqrt(12.0), 1.0 / sqrt(12.0) };

#pragma omp parallel for
    for (size_t iy = 0; iy < mesh.ny; ++iy) {
        size_t ic = iy * mesh.nx;
        for (size_t ix = 0; ix < mesh.nx; ++ix, ++ic) {
            Vertex xm = mesh.midpoint(ix, iy);
            for (unsigned short int gx = 0; gx < 2; ++gx)
                for (unsigned short int gy = 0; gy < 2; ++gy) {
                    // (f, phi0)
                    phi(ic, 0) += imass[0] * 0.25 * initial(xm[0] + mesh.hx * g2[gx], xm[1] + mesh.hy * g2[gy]);
                    phi(ic, 1) += imass[1] * 0.25 * initial(xm[0] + mesh.hx * g2[gx], xm[1] + mesh.hy * g2[gy]) * g2[gx];
                    phi(ic, 2) += imass[2] * 0.25 * initial(xm[0] + mesh.hx * g2[gx], xm[1] + mesh.hy * g2[gy]) * g2[gy];
                }
        }
    }
}

template <>
void L2ProjectInitial(const Mesh& mesh,
    CellVector<2>& phi,
    const InitialBase& initial)
{
    phi.setZero();

    std::array<double, 6> imass = { 1.0, 1.0, 1.0, 1., 1., 1. }; // without 'h'

    // Gauss quadrature on [-1/2, 1/2]
    std::array<double, 3> g3 = { -sqrt(3.0 / 5.0) * 0.5, 0.0, sqrt(3.0 / 5.0) * 0.5 };
    std::array<double, 3> w3 = { 5. / 18., 8. / 18., 5. / 18. };

#pragma omp parallel for
    for (size_t iy = 0; iy < mesh.ny; ++iy) {
        size_t ic = iy * mesh.nx;
        for (size_t ix = 0; ix < mesh.nx; ++ix, ++ic) {
            Vertex xm = mesh.midpoint(ix, iy);
            for (unsigned short int gx = 0; gx < 3; ++gx)
                for (unsigned short int gy = 0; gy < 3; ++gy) {
                    double x = xm[0] + mesh.hx * g3[gx];
                    double y = xm[1] + mesh.hy * g3[gy];

                    double X = 0.5 + g3[gx];
                    double Y = 0.5 + g3[gy];
                    // (f, phi0)
                    phi(ic, 0) += imass[0] * w3[gx] * w3[gy] * initial(x, y) * 1.0; // 1
                    phi(ic, 1) += imass[1] * w3[gx] * w3[gy] * initial(x, y) * (X - 0.5) * sqrt(12.0);
                    phi(ic, 2) += imass[2] * w3[gx] * w3[gy] * initial(x, y) * (Y - 0.5) * sqrt(12.0);
                    phi(ic, 3) += imass[3] * w3[gx] * w3[gy] * initial(x, y) * ((X - 0.5) * (X - 0.5) - 1. / 12.) * sqrt(180.0);
                    phi(ic, 4) += imass[4] * w3[gx] * w3[gy] * initial(x, y) * ((Y - 0.5) * (Y - 0.5) - 1. / 12.) * sqrt(180.0);
                    phi(ic, 5) += imass[5] * w3[gx] * w3[gy] * initial(x, y) * (X - 0.5) * (Y - 0.5) * 12.0;
                }
        }
    }
}

//! Functions to compute the error of a DG vector w.r.t. an analytical solution measured in L2

template <int DGdegree>
double L2Error(const Mesh& mesh,
    const CellVector<DGdegree>& phi,
    const InitialBase& initial);
template <>
double L2Error(const Mesh& mesh,
    const CellVector<2>& phi,
    const InitialBase& ex)
{
    double res = 0.0; //!< stores the integral

    // Gauss quadrature on [-1/2, 1/2]
    std::array<double, 3> g3 = { -sqrt(3.0 / 5.0) * 0.5, 0.0, sqrt(3.0 / 5.0) * 0.5 };
    std::array<double, 3> w3 = { 5. / 18., 8. / 18., 5. / 18. };

    double hxhy = mesh.hx * mesh.hy;

#pragma omp parallel for
    for (size_t iy = 0; iy < mesh.ny; ++iy) {
        size_t ic = iy * mesh.nx;
        for (size_t ix = 0; ix < mesh.nx; ++ix, ++ic) {
            Vertex xm = mesh.midpoint(ix, iy);
            for (unsigned short int gx = 0; gx < 3; ++gx)
                for (unsigned short int gy = 0; gy < 3; ++gy) {
                    double x = xm[0] + mesh.hx * g3[gx];
                    double y = xm[1] + mesh.hy * g3[gy];

                    double X = 0.5 + g3[gx];
                    double Y = 0.5 + g3[gy];

                    double PHI = phi(ic, 0) + phi(ic, 1) * (X - 0.5) * sqrt(12.0) + phi(ic, 2) * (Y - 0.5) * sqrt(12.0) + phi(ic, 3) * ((X - 0.5) * (X - 0.5) - 1.0 / 12) * sqrt(180.0) + phi(ic, 4) * ((Y - 0.5) * (Y - 0.5) - 1.0 / 12) * sqrt(180.0) + phi(ic, 5) * (X - 0.5) * (Y - 0.5) * 12.0;

#pragma omp atomic
                    res += w3[gx] * w3[gy] * hxhy * (PHI - ex(x, y)) * (PHI - ex(x, y));
                }
        }
    }
    return sqrt(res);
}

} // namespace Nextsim

/*----------------------------   initial.h     ---------------------------*/
/* end of #ifndef __initial_H */
#endif
/*----------------------------   initial.h     ---------------------------*/
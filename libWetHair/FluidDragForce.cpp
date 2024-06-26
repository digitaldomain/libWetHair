//
// This file is part of the libWetHair open source project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 Yun (Raymond) Fei, Henrique Teles Maia, Christopher Batty,
// Changxi Zheng, and Eitan Grinspun
//

#include "FluidDragForce.h"

#include <tbb/tbb.h>

#include "HairFlow.h"
#include "TwoDScene.h"
#include "array2_utils.h"
#include "array3_utils.h"
#include "fluidsim2D.h"
#include "fluidsim3D.h"

namespace libwethair {

template <int DIM>
FluidDragForce<DIM>::FluidDragForce(const TwoDScene<DIM>& scene, int hidx)
    : Force(), m_hidx(hidx), m_scene(scene) {
  const HairFlow<DIM>* flow = m_scene.getFilmFlows()[m_hidx];

  int np = flow->size();

  m_k0.resize(np);
  m_liquid_u.resize(np * DIM);
  m_k0.setZero();
  m_liquid_u.setZero();
}

template <int DIM>
FluidDragForce<DIM>::~FluidDragForce() {}

template <>
void FluidDragForce<2>::preCompute(const VectorXs& x, const VectorXs& v,
                                   const VectorXs& m, const scalar& dt) {
  const FluidSim2D* fluid2d = (const FluidSim2D*)m_scene.getFluidSim();

  auto& flows = m_scene.getFilmFlows();

  scalar rho = m_scene.getLiquidDensity();

  scalar visc = m_scene.getViscosity();

  scalar quaddrag = m_scene.getQuadraticDragging();

  m_k0.setZero();
  m_liquid_u.setZero();

  const scalar beta = 0.14288690166;

  HairFlow<2>* flow = flows[m_hidx];
  auto& radii_V = flow->getRadiiV();
  auto& dir_V = flow->getTangentV();
  auto& area_V = flow->getAreaV();
  auto& indices = flow->getParticleIndices();

  int numvert = indices.size();

  for (int j = 0; j < numvert; ++j) {
    int pidx = indices[j];
    const Vector2s& pos = x.segment<2>(pidx * 2);

    Vector2s vel = fluid2d->get_particle_velocity(pos);
    m_liquid_u.segment<2>(j * 2) = vel;

    scalar particle_weight = fluid2d->getClampedLiquidPhiValue(pos);
    if (particle_weight == 0.0)
      continue;

    Vector2s dv = v.segment<2>(pidx * 2) - vel;

    scalar ldv = dv.norm();

    if (ldv > 1e-7) {
      scalar prop = (radii_V(j) * M_PI) / (radii_V(j) * M_PI + area_V(j) * 2.0);

      Vector2s dir = dir_V.row(j).transpose().normalized();
      scalar len = area_V(j);
      scalar l = fabs(mathutils::cross2(dv, dir) / ldv) * (1.0 - prop) + prop;
      scalar radius = radii_V(j);

      scalar D = 6.0 * M_PI * l;

      const scalar b0 = visc * D * sqrt(len * radius * 2.0) * 0.5;
      const scalar b1 = quaddrag * len * radius * beta * sqrt(D) * rho;

      scalar ndv2 = dv.squaredNorm();
      scalar ndv = mathutils::softsqrt(ndv2, 1.0 / 200.0);

      scalar k0 = particle_weight * (b0 + b1 * ndv);
      m_k0(j) = k0;
    }
  }
}

template <>
void FluidDragForce<3>::preCompute(const VectorXs& x, const VectorXs& v,
                                   const VectorXs& m, const scalar& dt) {
  const FluidSim3D* fluid3d = (const FluidSim3D*)m_scene.getFluidSim();

  auto& flows = m_scene.getFilmFlows();

  scalar rho = m_scene.getLiquidDensity();

  scalar visc = m_scene.getViscosity();

  scalar quaddrag = m_scene.getQuadraticDragging();

  m_k0.setZero();
  m_liquid_u.setZero();

  const scalar beta = 0.14288690166;

  HairFlow<3>* flow = flows[m_hidx];
  auto& radii_V = flow->getRadiiV();
  auto& dir_V = flow->getTangentV();
  auto& area_V = flow->getAreaV();
  auto& indices = flow->getParticleIndices();
  auto& porosity_V = flow->getPorosity();

  int numvert = indices.size();

  for (int j = 0; j < numvert; ++j) {
    int pidx = indices[j];
    const Vector3s& pos = x.segment<3>(m_scene.getDof(pidx));

    Vector3s vel = fluid3d->get_particle_velocity(pos);
    m_liquid_u.segment<3>(3 * j) = vel;

    scalar clamped_particle_weight = fluid3d->getClampedLiquidPhiValue(pos);
    if (clamped_particle_weight == 0.0)
      continue;

    Vector3s dv = v.segment<3>(m_scene.getDof(pidx)) - vel;

    scalar ldv = dv.norm();

    if (ldv > 1e-7) {
      scalar prop = (radii_V(j) * M_PI) / (radii_V(j) * M_PI + area_V(j) * 2.0);

      Vector3s dir = dir_V.row(j).transpose().normalized();
      scalar len = area_V(j);
      scalar l = fabs(dv.cross(dir).norm() / ldv) * (1.0 - prop) + prop;
      scalar radius = radii_V(j);

      scalar D = 6.0 * M_PI * l;

      const scalar b0 = visc * D * sqrt(len * radius * 2.0) * 0.5;
      const scalar b1 = quaddrag * len * radius * beta * sqrt(D) * rho;

      scalar ndv2 = dv.squaredNorm();
      scalar ndv = mathutils::softsqrt(ndv2, 1.0 / 200.0);

      scalar k0 = clamped_particle_weight * (b0 + b1 * ndv);
      m_k0(j) = k0;
    }
  }
}

template <int DIM>
void FluidDragForce<DIM>::computeIntegrationVars(
    const VectorXs& x, const VectorXs& v, const VectorXs& m, VectorXs& lambda,
    VectorXs& lambda_v, TripletXs& J, TripletXs& Jv, TripletXs& Jxv,
    TripletXs& tildeK, TripletXs& stiffness, TripletXs& damping, VectorXs& Phi,
    VectorXs& Phiv, const scalar& dt) {
  const HairFlow<DIM>* flow = m_scene.getFilmFlows()[m_hidx];

  int np = flow->size();
  const std::vector<int>& particles = flow->getParticleIndices();

  const int nconstraints = m_hair_particle_affected.size();

  for (int k = 0; k < nconstraints; ++k) {
    int i = m_hair_particle_affected[k];
    int pidx = particles[i];

    Phiv.segment<DIM>(m_internal_index_vel + k * DIM) =
        v.segment<DIM>(m_scene.getDof(pidx)) - m_liquid_u.segment<DIM>(DIM * i);

    for (int r = 0; r < DIM; ++r) {
      damping[m_internal_index_vel + k * DIM + r] =
          Triplets(m_internal_index_vel + k * DIM + r,
                   m_internal_index_vel + k * DIM + r, m_k0(i));
      Jv[m_internal_index_Jv + k * DIM + r] = Triplets(
          m_internal_index_vel + k * DIM + r, m_scene.getDof(pidx) + r, 1.0);
    }
  };
}

template <int DIM>
int FluidDragForce<DIM>::numJ() {
  return 0;
}

template <int DIM>
int FluidDragForce<DIM>::numJv() {
  return m_hair_particle_affected.size() * DIM;
}

template <int DIM>
int FluidDragForce<DIM>::numJxv() {
  return 0;
}

template <int DIM>
int FluidDragForce<DIM>::numTildeK() {
  return 0;
}

template <int DIM>
bool FluidDragForce<DIM>::isParallelized() {
  return false;
}

template <int DIM>
bool FluidDragForce<DIM>::isPrecomputationParallelized() {
  return false;
}

template <int DIM>
int FluidDragForce<DIM>::numConstraintPos() {
  return 0;
}

template <int DIM>
int FluidDragForce<DIM>::numConstraintVel() {
  m_hair_particle_affected.resize(0);

  int numAffected = 0;
  const HairFlow<DIM>* flow = m_scene.getFilmFlows()[m_hidx];
  const std::vector<int>& particles = flow->getParticleIndices();
  int np = flow->size();

  for (int i = 0; i < np; ++i) {
    if (m_k0(i) != 0.0) {
      m_hair_particle_affected.push_back(i);
      numAffected++;
    }
  }
  return numAffected * DIM;
}

template <int DIM>
const char* FluidDragForce<DIM>::name() {
  return "fluiddragforce";
}

template <int DIM>
void FluidDragForce<DIM>::storeLambda(const VectorXs& lambda,
                                      const VectorXs& lambda_v) {}

template <int DIM>
Force* FluidDragForce<DIM>::createNewCopy() {
  return new FluidDragForce(*this);
}

template <int DIM>
void FluidDragForce<DIM>::getAffectedVars(int colidx,
                                          std::unordered_set<int>& vars) {
  vars.insert(colidx);
}

template <int DIM>
int FluidDragForce<DIM>::getAffectedHair(
    const std::vector<int> particle_to_hairs) {
  return m_hidx;
}

template <int DIM>
void FluidDragForce<DIM>::addGradEToTotal(const VectorXs& x, const VectorXs& v,
                                          const VectorXs& m, VectorXs& gradE) {
  assert(x.size() == v.size());
  assert(x.size() == m.size());
  assert(x.size() == gradE.size());

  const HairFlow<DIM>* flow = m_scene.getFilmFlows()[m_hidx];

  int np = flow->size();
  const std::vector<int>& particles = flow->getParticleIndices();

  for (int i = 0; i < np; ++i) {
    int pidx = particles[i];
    Vectors<DIM> dv =
        v.segment<DIM>(m_scene.getDof(pidx)) - m_liquid_u.segment<DIM>(DIM * i);
    scalar k = m_k0(i);
    gradE.segment<DIM>(m_scene.getDof(pidx)) += k * dv;
  }
}

template <int DIM>
bool FluidDragForce<DIM>::isContained(int colidx) {
  int idir = m_scene.getComponent(colidx);
  if (idir == DIM)
    return false;
  int pidx = m_scene.getVertFromDof(colidx);

  const std::vector<int>& particle_hairs = m_scene.getParticleToHairs();
  return particle_hairs[pidx] == m_hidx;
}

// explicit instantiations at bottom
template class FluidDragForce<2>;
template class FluidDragForce<3>;

}  // namespace libwethair

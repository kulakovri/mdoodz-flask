// =========================================================================
// MDOODZ - Visco-Elasto-Plastic Thermo-Mechanical solver
//
// Copyright (C) 2019  MDOODZ Developper team
//
// This file is part of MDOODZ.
//
// MDOODZ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MDOODZ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MDOODZ.  If not, see <http://www.gnu.org/licenses/>.
// =========================================================================

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "time.h"
#include "header_MDOODZ.h"

#ifdef _OMP_
#include "omp.h"
#else
#define omp_get_thread_num() 0
#define omp_get_num_threads() 1
#define omp_get_wtime() clock()/CLOCKS_PER_SEC
#endif

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void AddCoeff3( int* J, double*A, int eqn, int jeq, int *nnzc, double coeff, int NODE_TYPE, double NODE_VAL, double* RHS ) {

    if (NODE_TYPE == 0 || NODE_TYPE == 31 || NODE_TYPE == 11) { // || NODE_TYPE == 13
        RHS[eqn]  -= coeff * NODE_VAL;
    }
    else {
        if (jeq <= eqn ) {
            J[(*nnzc)] = jeq;
            A[(*nnzc)] = coeff;
            (*nnzc)++;
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void Continuity_InnerNodesDecoupled( SparseMat *Stokes, SparseMat *StokesC, SparseMat *StokesD, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempC, double **AtempC, int *nnzc2C, int **JtempD, double **AtempD, int *nnzc2D, int i, int j ) {

    double pc=0.0, uW=0.0, uE=0.0, vN=0.0, vS=0.0;

    // Compressibility
    if ( comp == 0 ) {
        pc = 0.0;
    }
    else {
        pc = mesh->bet_n[c2]/model.dt;
    }

    // div u
    if ( mesh->BCu.type[c1     ] != 13 ) uW = -one_dx * 1e0;
    if ( mesh->BCu.type[c1+1   ] != 13 ) uE =  one_dx * 1e0;
    if ( mesh->BCv.type[c3     ] != 13 ) vS = -one_dz * 1e0;
    if ( mesh->BCv.type[c3+nxvz] != 13 ) vN =  one_dz * 1e0;

    // Stencil assembly / residual
    if ( Assemble == 1 ) {
        StokesC->b[eqn] *= celvol;
        StokesD->b[eqn] *= celvol;
        if ( mesh->BCu.type[c1     ] != 13 )    AddCoeff2( JtempC[ith], AtempC[ith], eqn, Stokes->eqn_u[c1],      &(nnzc2C[ith]), uW*celvol, mesh->BCu.type[c1],      mesh->BCu.val[c1],      StokesC->bbc );
        if ( mesh->BCu.type[c1+1   ] != 13 )    AddCoeff2( JtempC[ith], AtempC[ith], eqn, Stokes->eqn_u[c1+1],    &(nnzc2C[ith]), uE*celvol, mesh->BCu.type[c1+1],    mesh->BCu.val[c1+1],    StokesC->bbc );
        if ( mesh->BCv.type[c3     ] != 13 )    AddCoeff2( JtempC[ith], AtempC[ith], eqn, Stokes->eqn_v[c3],      &(nnzc2C[ith]), vS*celvol, mesh->BCv.type[c3],      mesh->BCv.val[c3],      StokesC->bbc );
        if ( mesh->BCv.type[c3+nxvz] != 13 )    AddCoeff2( JtempC[ith], AtempC[ith], eqn, Stokes->eqn_v[c3+nxvz], &(nnzc2C[ith]), vN*celvol, mesh->BCv.type[c3+nxvz], mesh->BCv.val[c3+nxvz], StokesC->bbc );
    }
    else {
        
        // d ln rho
//        dlnrhodt = (log(rhoc) - log(rhoc0)) /dt;
//        if comp==1, fp = -divc - dlnrhodt; resnlp = norm(fp(:))/length(fp); end
        //                                               = beta*(p - p0)/dt + div(v)
        if ( model.VolChangeReac == 0 ) StokesC->F[eqn] =                      pc*p[c2] + uW*u[c1] + uE*u[c1+1] + vS*v[c3] + vN*v[c3+nxvz];
        //                                               = d(ln(rho))/dt + div(v)
        if ( model.VolChangeReac == 1 ) StokesC->F[eqn] = log(mesh->rho_n[c2])/model.dt + uW*u[c1] + uE*u[c1+1] + vS*v[c3] + vN*v[c3+nxvz];
        StokesC->F[eqn] -= StokesC->b[eqn];
        StokesC->F[eqn] *= celvol;
        StokesD->F[eqn] = StokesC->F[eqn] ;

    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void Xmomentum_InnerNodesDecoupled( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B  ) {

    int iVxC   = c1;
    int iVxW   = iVxC-1;
    int iVxE   = iVxC+1;
    int iVxS   = iVxC-nx;
    int iVxN   = iVxC+nx;
    int iVzSW  = c3-nxvz;
    int iVzSE  = c3-nxvz+1;
    int iVzNW  = c3;
    int iVzNE  = c3+1;
    int iPrW   = c2;
    int iPrE   = c2+1;
    int ixyN   = c1;
    int ixyS   = c1-nx;

    // Periodic stencil
    if ( mesh->BCu.type[iVxC]==-2 ) {
        iVxW  = c1+nx-2;
        iPrW  = c2+ncx;
        iVzSW = c3-2;
        iVzNW = c3+nxvz-2;
    }

    double D11E = mesh->D11_n[iPrE];
    double D11W = mesh->D11_n[iPrW];
    double D33N = mesh->D33_s[ixyN];
    double D33S = mesh->D33_s[ixyS];

    double uS=0.0, uN=0.0, uW=0.0, uE=0.0, uC=0.0, vSW=0.0, vSE=0.0, vNW=0.0, vNE=0.0, pE=0.0, pW=0.0;
    double inE=0.0, inW=0.0, inSu=0.0, inSv=0.0, inNu=0.0, inNv=0.0;
    if (mesh->BCp.type[iPrW] == -1) inW = 1.0;
    if (mesh->BCp.type[iPrE] == -1) inE = 1.0;

    double inS=0.0, inN = 0.0;
    if (mesh->BCg.type[ixyS] != 30 && mesh->BCu.type[ixyS] != 13) inS = 1.0;
    if (mesh->BCg.type[c1  ] != 30 && mesh->BCu.type[c1+nx] != 13) inN = 1.0; /// ?????????????????????????

    double dx = mesh->dx;
    double dz = mesh->dz;

    comp = 1.0;

    // simpler
    uW = D11W*inW*((1.0/3.0)*comp/dx - 1.0/dx)/dx;
    uC = -(-D33N*inN/dz - D33S*inS/dz)/dz - (D11E*inE*((1.0/3.0)*comp/dx - 1/dx) - D11W*inW*(-1.0/3.0*comp/dx + 1.0/dx))/dx;
    uE = -D11E*inE*(-1.0/3.0*comp/dx + 1.0/dx)/dx;
    uS = -D33S*inS/pow(dz, 2.0);
    uN = -D33N*inN/pow(dz, 2.0);
    vSW = (1.0/3.0)*D11W*comp*inW/(dx*dz) - D33S*inS/(dx*dz);
    vSE = -1.0/3.0*D11E*comp*inE/(dx*dz) + D33S*inS/(dx*dz);
    vNW = -1.0/3.0*D11W*comp*inW/(dx*dz) + D33N*inN/(dx*dz);
    vNE = (1.0/3.0)*D11E*comp*inE/(dx*dz) - D33N*inN/(dx*dz);
    
    // Stabilisation with density gradients
    if (stab==1) {
        double drhodx  = (mesh->rho_n[c2+1] - mesh->rho_n[c2])*one_dx;
        double uC_corr = 1.00 * om * model.dt * mesh->gx[c1] * drhodx;
        // Importante trique, voire meme gigantesque!
        if (uC+uC_corr>0.0) uC += uC_corr;
    }

    // Add contribution from non-conforming Dirichlets
    if ( mesh->BCu.type[iVxS] == 11 ) uC  -=  uS;
    if ( mesh->BCu.type[iVxN] == 11 ) uC  -=  uN;

    // Pressure gradient
    if ( mesh->BCp.type[iPrE] != 30 && mesh->BCp.type[iPrW] != 30 ) {
        pW  =  -one_dx * 1e0;
        pE  =   one_dx * 1e0;
    }

    if ( Assemble == 1 ) {
        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------
        // dsxx/dx - normal stencil
        if (mesh->BCu.type[iVxS] != 30) {
            if (mesh->BCu.type[iVxS] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxS], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[iVxS],     mesh->BCu.val[iVxS], StokesA->bbc);
            else                            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxS], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[iVxS], 2.0*mesh->BCu.val[iVxS], StokesA->bbc);
        }
        if (mesh->BCu.type[iVxW]  != 30)    AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxW],  &(nnzc2A[ith]), uW*celvol, mesh->BCu.type[iVxW],  mesh->BCu.val[iVxW],  StokesA->bbc);
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                  &(nnzc2A[ith]), uC*celvol, mesh->BCu.type[iVxC],    mesh->BCu.val[iVxC],    StokesA->bbc);
        if (mesh->BCu.type[iVxE]  != 30)    AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxE],  &(nnzc2A[ith]), uE*celvol, mesh->BCu.type[iVxE],  mesh->BCu.val[iVxE],  StokesA->bbc);
        if (mesh->BCu.type[c1+nx] != 30) {
            if (mesh->BCu.type[iVxN] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxN], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[iVxN],   mesh->BCu.val[iVxN], StokesA->bbc);
            else                            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxN], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[iVxN], 2*mesh->BCu.val[iVxN], StokesA->bbc);
        }
        //--------------------
        if ( mesh->BCv.type[iVzSW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSW], &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[iVzSW], mesh->BCv.val[iVzSW], StokesA->bbc);
        if ( mesh->BCv.type[iVzSE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSE], &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[iVzSE], mesh->BCv.val[iVzSE], StokesA->bbc);
        if ( mesh->BCv.type[iVzNW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNW], &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[iVzNW], mesh->BCv.val[iVzNW], StokesA->bbc);
        if ( mesh->BCv.type[iVzNE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNE], &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[iVzNE], mesh->BCv.val[iVzNE], StokesA->bbc);

        //--------------------
        if ( mesh->BCp.type[iPrE] != 30 && mesh->BCp.type[iPrW] != 30 ) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrW] - Stokes->neq_mom, &(nnzc2B[ith]), pW*celvol, mesh->BCp.type[iPrW], mesh->BCp.val[iPrW], StokesB->bbc);
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrE] - Stokes->neq_mom, &(nnzc2B[ith]), pE*celvol, mesh->BCp.type[iPrE], mesh->BCp.val[iPrE], StokesB->bbc);
        }

    }
    else {

        // Residual function
        StokesA->F[eqn] = uC*u[iVxC];
        if ( mesh->BCp.type[iPrE] != 30 && mesh->BCp.type[iPrW] != 30 ) {
            StokesA->F[eqn]  += pW*p[iPrW] + pE*p[iPrE];
        }
        if ( mesh->BCv.type[iVzSE] != 30 )  StokesA->F[eqn] += vSE*v[iVzSE];
        if ( mesh->BCv.type[iVzSW] != 30 )  StokesA->F[eqn] += vSW*v[iVzSW];
        if ( mesh->BCv.type[iVzNE] != 30 )  StokesA->F[eqn] += vNE*v[iVzNE];
        if ( mesh->BCv.type[iVzNW] != 30 )  StokesA->F[eqn] += vNW*v[iVzNW];
        if ( mesh->BCu.type[iVxS]  != 30 && mesh->BCu.type[iVxS] != 11  ) StokesA->F[eqn] += uS*u[iVxS];
//        if (                                mesh->BCu.type[iVxS] == 11  ) StokesA->F[eqn] += -2.0*D33S*one_dz_dz*mesh->BCu.val[iVxS];
        if (                                mesh->BCu.type[iVxS] == 11  ) StokesA->F[eqn] += 2.0*uS*mesh->BCu.val[iVxS];
        if ( mesh->BCu.type[iVxN]  != 30 && mesh->BCu.type[iVxN] != 11  ) StokesA->F[eqn] += uN*u[iVxN];
//        if (                                mesh->BCu.type[iVxN] == 11  ) StokesA->F[eqn] += -2.0*D33N*one_dz_dz*mesh->BCu.val[iVxN];
        if (                                mesh->BCu.type[iVxN] == 11  ) StokesA->F[eqn] += 2.0*uN*mesh->BCu.val[iVxN];
        if ( mesh->BCu.type[iVxW]  != 30 ) StokesA->F[eqn] += uW*u[iVxW];
        if ( mesh->BCu.type[iVxE]  != 30 ) StokesA->F[eqn] += uE*u[iVxE];
        
        StokesA->F[eqn] -= (StokesA->b[eqn]);// + Stokes->bbc[eqn];
        StokesA->F[eqn] *= celvol;
//        if (isnan(StokesA->F[eqn]) ) printf("cuC = %2.2e u[iVxC] = %2.2e, D11W = %2.2e, D11E = %2.2e D33S = %2.2e D33N = %2.2e \n", uC, u[iVxC],  D11W, D11E, D33S, D33N );
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

//void Xmomentum_InnerNodesDecoupled_v4.5( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B  ) {
//
//    double AE = mesh->eta_n[c2+1];
//    double AW = mesh->eta_n[c2];
//    double AN = mesh->eta_s[c1];
//    double AS = mesh->eta_s[c1-nx];
//
//    double uS=0.0, uN=0.0, uW=0.0, uE=0.0, uC=0.0, vSW=0.0, vSE=0.0, vNW=0.0, vNE=0.0, pE=0.0, pW=0.0;
//
//    // dsxx/dx
//    if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1+1] != 30 ) {
//        uW  =  2*one_dx_dx * AW;//         - comp*2.0/3.0*one_dx_dx * AW;
//        uC  = -2*one_dx_dx * (AE + AW);//  + comp*2.0/3.0*one_dx_dx * AE + comp*2.0/3.0*one_dx_dx * AW;
//        uE  =  2*one_dx_dx * AE;//         - comp*2.0/3.0*one_dx_dx * AE;
//    }
//    if ( mesh->BCu.type[c1-1] == 30 && mesh->BCu.type[c1+1] != 30  ) {
//        uC  = -2*one_dx_dx * (AE) + comp*2.0/3.0*one_dx_dx * AE;
//        uE  =  2*one_dx_dx * AE - comp*2.0/3.0*one_dx_dx * AE;
//    }
//    if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1+1] == 30 ) {
//        uW  =  2*one_dx_dx * AW - comp*2.0/3.0*one_dx_dx * AW;
//        uC  = -2*one_dx_dx * (AW) + comp*2.0/3.0*one_dx_dx * AW;
//    }
//
//    // eta*dvx/dz
//    if ( mesh->BCu.type[c1-nx] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
//        if ( mesh->BCu.type[c1-nx] != 13 ) {uS   =  one_dz_dz * AS; uC  +=  -one_dz_dz * AS;}
//        if ( mesh->BCu.type[c1+nx] != 13 ) {uN   =  one_dz_dz * AN; uC  +=  -one_dz_dz * AN;}
//    }
//    if ( mesh->BCu.type[c1-nx] == 30 && mesh->BCu.type[c1+nx] != 30 ) {
//        if ( mesh->BCu.type[c1+nx] != 13 ) uC  +=  -one_dz_dz * AN;
//        if ( mesh->BCu.type[c1+nx] != 13 ) uN   =   one_dz_dz * AN;
//    }
//    if ( mesh->BCu.type[c1-nx] != 30 && mesh->BCu.type[c1+nx] == 30 ) {
//        if ( mesh->BCu.type[c1-nx] != 13 ) uC  +=  -one_dz_dz * AS;
//        if ( mesh->BCu.type[c1-nx] != 13 ) uS   =   one_dz_dz * AS;
//    }
//
//    if ( mesh->BCu.type[c1-nx] == 11 ) uC  +=  -one_dz_dz * AS;
//    if ( mesh->BCu.type[c1+nx] == 11 ) uC  +=  -one_dz_dz * AN;
//
//    // eta*dvz/dx
//    if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3-nxvz+1] != 30 )  {
//        vSE = -one_dx_dz * AS + comp*2.0/3.0*one_dx_dz * AE;
//        vSW =  one_dx_dz * AS - comp*2.0/3.0*one_dx_dz * AW;
//    }
//
//    if ( mesh->BCv.type[c3+1] != 30 && mesh->BCv.type[c3] != 30  ) {
//        vNE =  one_dx_dz * AN - comp*2.0/3.0*one_dx_dz * AE;
//        vNW = -one_dx_dz * AN + comp*2.0/3.0*one_dx_dz * AW;
//    }
//
//    // Pressure gradient
//    if ( mesh->BCp.type[c2+1] != 30 && mesh->BCp.type[c2] != 30 ) {
//        pW  =   one_dx;
//        pE  =  -pW;
//    }
//
//    // Pressure gradient
//    //    if ( mesh->BCp.type[c2+1] != 30 ) pW  =   one_dx;
//    //    if ( mesh->BCp.type[c2]   != 30 ) pE  =  -one_dx;
//
//    //    // Inertia
//    //    if ( model.isinertial == 1 || model.isinertial == 2 ) {
//    //        uC -= sign * rhoVx/model.dt;
//    //    }
//    //    if ( model.isinertial == 2 ) {
//    //        uN -= rhoVx/2*one_dz*mesh->VzVx[c1];
//    //        uS += rhoVx/2*one_dz*mesh->VzVx[c1];
//    //        uW += rhoVx/2*one_dx*mesh->u_in[c1];
//    //        uE -= rhoVx/2*one_dx*mesh->u_in[c1];
//    //    }
//
//    //    // Stabilisation with density gradients
//    //    if ( stab == 1 ) {
//    ////        double correction = - om*0.5*model.dt * model.gx * (mesh->rho_n[c2+1] - mesh->rho_n[c2]) * one_dx;
//    ////        uC += correction;
//    ////        correction = - om*0.5*model.dt * model.gx * (mesh->rho_s[c1] - mesh->rho_s[c1-nx]) * one_dz;
//    ////        vSW += 0.25*correction;
//    ////        vNE += 0.25*correction;
//    ////        vNW += 0.25*correction;
//    ////        vSE += 0.25*correction;
//    //    }
//
//    // Stabilisation with density gradients
//    if (stab==1) {
//        double drhodx  = (mesh->rho_n[c2+1] - mesh->rho_n[c2])*one_dx;
//        double uC_corr = - 1.00 * om * model.dt * model.gx * drhodx;
//        // Importante trique, voire meme gigantesque!
//                if (uC+uC_corr<0.0)  uC += uC_corr;
//        //        if (vW+.25*vC_corr>0)  vW += .25*vC_corr;
//        //        if (vE+.25*vC_corr>0)  vE += .25*vC_corr;
//        //        if (vS+.25*vC_corr>0)  vS += .25*vC_corr;
//        //        if (vN+.25*vC_corr>0)  vN += .25*vC_corr;
//
//        // Non-symmetric contibution to the system of equation
//        //        double drhodx = (mesh->rho_s[c1]   - mesh->rho_s[c1-1])*one_dx;
//        //        uNW += - 0.25 * om * model.dt * model.gz * drhodx;
//        //        uNE += - 0.25 * om * model.dt * model.gz * drhodx;
//        //        uSW += - 0.25 * om * model.dt * model.gz * drhodx;
//        //        uSE += - 0.25 * om * model.dt * model.gz * drhodx;
//    }
//
//    uS=-uS, uN=-uN, uW=-uW, uE=-uE, uC=-uC, vSW=-vSW, vSE=-vSE, vNW=-vNW, vNE=-vNE, pE=-pE, pW=-pW;
//
//    if ( Assemble == 1 ) {
//        StokesA->b[eqn] *= celvol;
//        StokesB->b[eqn] *= celvol;
//        //--------------------
//        // dsxx/dx - normal stencil
//        if (mesh->BCu.type[c1-nx] != 30) {
//            if (mesh->BCu.type[c1-nx] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-nx], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[c1-nx], mesh->BCu.val[c1-nx], StokesA->bbc);
//            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-nx], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[c1-nx], 2.0*mesh->BCu.val[c1-nx], StokesA->bbc);
//        }
//        if (mesh->BCu.type[c1-1]  != 30)     AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-1],  &(nnzc2A[ith]), uW*celvol, mesh->BCu.type[c1-1],  mesh->BCu.val[c1-1],  StokesA->bbc);
//        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                  &(nnzc2A[ith]), uC*celvol, mesh->BCu.type[c1],    mesh->BCu.val[c1],    StokesA->bbc);
//        if (mesh->BCu.type[c1+1]  != 30)     AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+1],  &(nnzc2A[ith]), uE*celvol, mesh->BCu.type[c1+1],  mesh->BCu.val[c1+1],  StokesA->bbc);
//        if (mesh->BCu.type[c1+nx] != 30) {
//            if (mesh->BCu.type[c1+nx] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[c1+nx], mesh->BCu.val[c1+nx], StokesA->bbc);
//            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[c1+nx], 2*mesh->BCu.val[c1+nx], StokesA->bbc);
//        }
//        //--------------------
//
//        if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3-nxvz+1] != 30 )  {
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz],   &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[c3-nxvz],   mesh->BCv.val[c3-nxvz],   StokesA->bbc);
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz+1], &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[c3-nxvz+1], mesh->BCv.val[c3-nxvz+1], StokesA->bbc);
//        }
//
//        if ( mesh->BCv.type[c3] != 30 && mesh->BCv.type[c3+1] != 30 )  {
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3],        &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[c3],        mesh->BCv.val[c3],        StokesA->bbc);
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],      &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[c3+1],      mesh->BCv.val[c3+1],      StokesA->bbc);
//        }
//
//        //--------------------
//        if ( mesh->BCp.type[c2+1] != 30 && mesh->BCp.type[c2] != 30 ) {
//            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2] - Stokes->neq_mom,   &(nnzc2B[ith]), pW*celvol, mesh->BCp.type[c2],   mesh->BCp.val[c2],   StokesB->bbc);
//            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2+1] - Stokes->neq_mom, &(nnzc2B[ith]), pE*celvol, mesh->BCp.type[c2+1], mesh->BCp.val[c2+1], StokesB->bbc);
//        }
//        //        if ( mesh->BCp.type[c2+1] != 30 ) AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2] - Stokes->neq_mom,   &(nnzc2B[ith]), pW*celvol, mesh->BCp.type[c2],   mesh->BCp.val[c2],   StokesB->bbc);
//        //        if ( mesh->BCp.type[c2]   != 30 ) AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2+1] - Stokes->neq_mom, &(nnzc2B[ith]), pE*celvol, mesh->BCp.type[c2+1], mesh->BCp.val[c2+1], StokesB->bbc);
//
//        //        if (eqn==35931) {
//        //
//        //            printf("x momentum --  AW = %2.2e AE = %2.2e AS = %2.2e AN = %2.2e\n", AW, AE, AS, AN);
//        //            printf("uC = %02d %2.2e %2.12e\n",     mesh->BCu.type[c1+0],  uC, u[c1] );
//        //            printf("uW = %02d %2.2e %2.12e\n",     mesh->BCu.type[c1-1],  uW, u[c1-1] );
//        //            printf("uE = %02d %2.2e %2.2e\n",      mesh->BCu.type[c1+1],  uE, u[c1+1] );
//        //            printf("uS = %02d %2.2e %2.2e \n",     mesh->BCu.type[c1-nx], uS, u[c1-nx] );
//        //            printf("uN = %02d %2.2e %2.2e \n",     mesh->BCu.type[c1+nx], uN, u[c1+nx] );
//        //            printf("vSE= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3-nxvz+1], vSE, v[c3-nxvz+1] );
//        //            printf("vNE= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3+1], vNE, v[c3+1] );
//        //            printf("vSW= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3-nxvz], vSW, v[c3-nxvz] );
//        //            printf("vNW= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3], vNW, v[c3] );
//        //            printf("pW = %02d %2.2e %2.2e \n",     mesh->BCp.type[c2], pW, p[c2] );
//        //            printf("pE = %02d %2.2e %2.2e \n",     mesh->BCp.type[c2+1], pE, p[c2+1] );
//        //
//        //        }
//
//
//
//        /*
//         if (eqn==7656) {
//         printf("\nFx = %2.2e %d\n",StokesA->F[eqn]*4e-5, mesh->BCu.type[c1] );
//
//
//         ////        double F1 = pW*p[c2] + pE*p[c2+1] + vSW*v[c3-nxvz] + vSE*v[c3-nxvz+1] + vNW*v[c3] + vNE*v[c3+1] + uS*u[c1-nx] + uN*u[c1+nx] + uW*u[c1-1] + uE*u[c1+1] + uC*u[c1] + (StokesA->b[eqn] - StokesA->bbc[eqn]);
//         ////        if (fabs(F1)>1e-6) {
//         ////        printf("F0 = %2.2e %2.2e\n", StokesA->F[eqn], F1*celvol);
//         printf("x momentum --  AW = %2.2e AE = %2.2e AS = %2.2e AN = %2.2e\n", AW, AE, AS, AN);
//         printf("uC = %02d %2.2e %2.12e\n",      mesh->BCu.type[c1+0],  uC, u[c1] );
//         printf("uW = %02d %2.2e %2.12e\n",      mesh->BCu.type[c1-1],  uW, u[c1-1] );
//         printf("uE = %02d %2.2e %2.2e\n",      mesh->BCu.type[c1+1],  uE, u[c1+1] );
//         printf("uS = %02d %2.2e %2.2e \n",     mesh->BCu.type[c1-nx], uS, u[c1-nx] );
//         printf("uN = %02d %2.2e %2.2e \n",     mesh->BCu.type[c1+nx], uN, u[c1+nx] );
//         printf("vSE= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3-nxvz+1], vSE, v[c3-nxvz+1] );
//         printf("vNE= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3+1], vNE, v[c3+1] );
//         printf("vSW= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3-nxvz], vSW, v[c3-nxvz] );
//         printf("vNW= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3], vNW, v[c3] );
//         printf("pW = %02d %2.2e %2.2e \n",     mesh->BCp.type[c2], pW, p[c2] );
//         printf("pE = %02d %2.2e %2.2e %2.2e %2.2e bbc = %2.2e b = %2.2e\n",mesh->BCp.type[c2+1], pE, p[c2+1], StokesA->b[eqn] + StokesA->bbc[eqn], mesh->BCu.val[c1], StokesA->bbc[eqn] , StokesA->b[eqn]);
//         printf("%2.2e EQN=%d\n", (uC*u[c1] + uW*u[c1-1])*4e-5* celvol, eqn);
//         }
//         */
//
//    }
//    else {
//        // Residual function
//        StokesA->F[eqn] = uC*u[c1];
//        if ( mesh->BCp.type[c2+1] != 30 && mesh->BCp.type[c2] != 30 ) {
//            StokesA->F[eqn]  += pW*p[c2] + pE*p[c2+1];
//        }
//        if ( mesh->BCv.type[c3-nxvz+1] != 30 && mesh->BCv.type[c3-nxvz] != 30  ) {
//            StokesA->F[eqn] += vSW*v[c3-nxvz] + vSE*v[c3-nxvz+1];
//        }
//        if ( mesh->BCv.type[c3+1] != 30 && mesh->BCv.type[c3] != 30 ) {
//            StokesA->F[eqn] += vNW*v[c3] + vNE*v[c3+1];
//        }
//        if ( mesh->BCu.type[c1-nx] != 30 && mesh->BCu.type[c1-nx] != 11  ) StokesA->F[eqn] += uS*u[c1-nx];
//        if ( mesh->BCu.type[c1+nx] != 30 && mesh->BCu.type[c1+nx] != 11  ) StokesA->F[eqn] += uN*u[c1+nx];
//        if ( mesh->BCu.type[c1-1]  != 30 ) StokesA->F[eqn] += uW*u[c1-1];
//        if ( mesh->BCu.type[c1+1]  != 30 ) StokesA->F[eqn] += uE*u[c1+1];
//        if ( mesh->BCu.type[c1-nx] == 11 ) StokesA->F[eqn] += -2.0*AS*one_dz_dz*mesh->BCu.val[c1-nx];
//        if ( mesh->BCu.type[c1+nx] == 11 ) StokesA->F[eqn] += -2.0*AN*one_dz_dz*mesh->BCu.val[c1+nx];
//        StokesA->F[eqn] -= (StokesA->b[eqn]);// + Stokes->bbc[eqn];
//        StokesA->F[eqn] *= celvol;
//
//        /*
//         if (eqn==7656) {
//         printf("\nFx = %2.2e %d\n",StokesA->F[eqn]*4e-5, mesh->BCu.type[c1] );
//
//         ////        double F1 = pW*p[c2] + pE*p[c2+1] + vSW*v[c3-nxvz] + vSE*v[c3-nxvz+1] + vNW*v[c3] + vNE*v[c3+1] + uS*u[c1-nx] + uN*u[c1+nx] + uW*u[c1-1] + uE*u[c1+1] + uC*u[c1] + (StokesA->b[eqn] - StokesA->bbc[eqn]);
//         ////        if (fabs(F1)>1e-6) {
//         ////        printf("F0 = %2.2e %2.2e\n", StokesA->F[eqn], F1*celvol);
//         printf("x momentum --  AW = %2.2e AE = %2.2e AS = %2.2e AN = %2.2e\n", AW, AE, AS, AN);
//         printf("uC = %02d %2.2e %2.12e\n",      mesh->BCu.type[c1+0],  uC, u[c1] );
//         printf("uW = %02d %2.2e %2.12e\n",      mesh->BCu.type[c1-1],  uW, u[c1-1] );
//         printf("uE = %02d %2.2e %2.2e\n",      mesh->BCu.type[c1+1],  uE, u[c1+1] );
//         printf("uS = %02d %2.2e %2.2e \n",     mesh->BCu.type[c1-nx], uS, u[c1-nx] );
//         printf("uN = %02d %2.2e %2.2e \n",     mesh->BCu.type[c1+nx], uN, u[c1+nx] );
//         printf("vSE= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3-nxvz+1], vSE, v[c3-nxvz+1] );
//         printf("vNE= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3+1], vNE, v[c3+1] );
//         printf("vSW= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3-nxvz], vSW, v[c3-nxvz] );
//         printf("vNW= %02d %2.2e %2.2e \n",     mesh->BCv.type[c3], vNW, v[c3] );
//         printf("pW = %02d %2.2e %2.2e \n",     mesh->BCp.type[c2], pW, p[c2] );
//         printf("pE = %02d %2.2e %2.2e %2.2e %2.2e bbc = %2.2e b = %2.2e\n",mesh->BCp.type[c2+1], pE, p[c2+1], StokesA->b[eqn] + StokesA->bbc[eqn], mesh->BCu.val[c1], StokesA->bbc[eqn] , StokesA->b[eqn]);
//         printf("%2.2e EQN=%d\n", (uC*u[c1] + uW*u[c1-1])*4e-5* celvol, eqn);
//         }
//         */
//    }
//}


/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void Xmomentum_WestNeumannDecoupled( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B  ) {

    double etaE, etaW, etaN, etaS;
    etaE = mesh->eta_n[c2+1];
    etaW = etaE;
    etaN = mesh->eta_s[c1];
    etaS = mesh->eta_s[c1-nx];


    //    double rhoVx = 0.5*(mesh->rho_s[c1] + mesh->rho_s[c1-nx]);

    double uS=0.0, uN=0.0, uW=0.0, uE=0.0, uC=0.0, vSW=0.0, vSE=0.0, vNW=0.0, vNE=0.0, pE=0.0, pW=0.0;

    StokesA->b[eqn] -= mesh->BCu.val[c1]*one_dx;

    // dsxx/dx
    if ( mesh->BCu.type[c1+1] != 30 ) {
        uC  = -2.0*one_dx_dx * (etaE)  + comp*2.0/3.0*one_dx_dx * etaE;
        uE  =  2.0*one_dx_dx * etaE    - comp*2.0/3.0*one_dx_dx * etaE;
    }

    // eta*dvx/dz
    if ( mesh->BCu.type[c1-nx] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
        if ( mesh->BCu.type[c1-nx] != 13 ) {uS   =  one_dz_dz * etaS; uC  +=  -one_dz_dz * etaS;}
        if ( mesh->BCu.type[c1+nx] != 13 ) {uN   =  one_dz_dz * etaN; uC  +=  -one_dz_dz * etaN;}
    }
    if ( mesh->BCu.type[c1-nx] == 30 && mesh->BCu.type[c1+nx] != 30 ) {
        if ( mesh->BCu.type[c1+nx] != 13 ) uC  +=  -one_dz_dz * etaN;
        if ( mesh->BCu.type[c1+nx] != 13 ) uN   =   one_dz_dz * etaN;
    }
    if ( mesh->BCu.type[c1-nx] != 30 && mesh->BCu.type[c1+nx] == 30 ) {
        if ( mesh->BCu.type[c1-nx] != 13 ) uC  +=  -one_dz_dz * etaS;
        if ( mesh->BCu.type[c1-nx] != 13 ) uS   =   one_dz_dz * etaS;
    }

    if ( mesh->BCu.type[c1-nx] == 11 ) uC  +=  -one_dz_dz * etaS;
    if ( mesh->BCu.type[c1+nx] == 11 ) uC  +=  -one_dz_dz * etaN;

    // eta*dvz/dx
    if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3-nxvz+1] != 30 )  {
        vSE = -one_dx_dz * etaS + comp*2.0/3.0*one_dx_dz * etaE;
        vSW =  one_dx_dz * etaS - comp*2.0/3.0*one_dx_dz * etaW;
    }

    if ( mesh->BCv.type[c3+1] != 30 && mesh->BCv.type[c3] != 30  ) {
        vNE =  one_dx_dz * etaN - comp*2.0/3.0*one_dx_dz * etaE;
        vNW = -one_dx_dz * etaN + comp*2.0/3.0*one_dx_dz * etaW;
    }

    // Pressure gradient
    if ( mesh->BCp.type[c2+1] != 30 ) {
        pW  =   one_dx;
        pE  =  -pW;
    }
    //
    //    // Inertia
    //    if ( model.isinertial == 1 || model.isinertial == 2 ) {
    //        uC -= sign * rhoVx/model.dt;
    //    }
    //    if ( model.isinertial == 2 ) {
    //        uN -= rhoVx/2*one_dz*mesh->VzVx[c1];
    //        uS += rhoVx/2*one_dz*mesh->VzVx[c1];
    //        uW += rhoVx/2*one_dx*mesh->u_in[c1];
    //        uE -= rhoVx/2*one_dx*mesh->u_in[c1];
    //    }

    uS=-uS, uN=-uN, uW=-uW, uE=-uE, uC=-uC, vSW=-vSW, vSE=-vSE, vNW=-vNW, vNE=-vNE, pE=-pE, pW=-pW;

    if ( Assemble == 1 ) {
        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------
        // dsxx/dx - normal stencil
        if (mesh->BCu.type[c1-nx] != 30) {
            if (mesh->BCu.type[c1-nx] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-nx], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[c1-nx], mesh->BCu.val[c1-nx], StokesA->bbc);
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-nx], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[c1-nx], 2.0*mesh->BCu.val[c1-nx], StokesA->bbc);
        }
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                  &(nnzc2A[ith]), uC*celvol, mesh->BCu.type[c1],    mesh->BCu.val[c1],    StokesA->bbc);
        if (mesh->BCu.type[c1+1]  != 30)     AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+1],  &(nnzc2A[ith]), uE*celvol, mesh->BCu.type[c1+1],  mesh->BCu.val[c1+1],  StokesA->bbc);
        if (mesh->BCu.type[c1+nx] != 30) {
            if (mesh->BCu.type[c1+nx] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[c1+nx], mesh->BCu.val[c1+nx], StokesA->bbc);
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[c1+nx], 2*mesh->BCu.val[c1+nx], StokesA->bbc);
        }
        //--------------------

        if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3-nxvz+1] != 30 )  {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz],   &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[c3-nxvz],   mesh->BCv.val[c3-nxvz],   StokesA->bbc);
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz+1], &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[c3-nxvz+1], mesh->BCv.val[c3-nxvz+1], StokesA->bbc);
        }

        if ( mesh->BCv.type[c3] != 30 && mesh->BCv.type[c3+1] != 30 )  {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3],        &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[c3],        mesh->BCv.val[c3],        StokesA->bbc);
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],      &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[c3+1],      mesh->BCv.val[c3+1],      StokesA->bbc);
        }
        //--------------------
        if ( mesh->BCp.type[c2+1] != 30  ) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2+1] - Stokes->neq_mom, &(nnzc2B[ith]), pE*celvol, mesh->BCp.type[c2+1], mesh->BCp.val[c2+1], StokesB->bbc);
        }
    }
    else {

        // RHS
        StokesA->F[eqn] = -(StokesA->b[eqn] + StokesA->bbc[eqn]/celvol);
        StokesA->F[eqn] += uC*u[c1];
        if (mesh->BCu.type[c1-nx] != 30) {
            if (mesh->BCu.type[c1-nx] != 11) StokesA->F[eqn] += uS*u[c1-nx];
        }
        if (mesh->BCu.type[c1+1]  != 30)     StokesA->F[eqn] += uE*u[c1+1];
        if (mesh->BCu.type[c1+nx] != 30) {
            if (mesh->BCu.type[c1+nx] != 11) StokesA->F[eqn] += uN*u[c1+nx];
        }
        //--------------------
        if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3-nxvz+1] != 30 )  {
            if (mesh->BCv.type[c3-nxvz+1] != 11 && mesh->BCv.type[c3-nxvz+1] != 0) StokesA->F[eqn] += vSE*v[c3-nxvz+1];
            if (mesh->BCv.type[c3-nxvz  ] != 11 && mesh->BCv.type[c3-nxvz  ] != 0) StokesA->F[eqn] += vSW*v[c3-nxvz];
        }
        if ( mesh->BCv.type[c3] != 30 && mesh->BCv.type[c3+1] != 30 )  {
            if (mesh->BCv.type[c3+1] != 11 && mesh->BCv.type[c3+1] != 0) StokesA->F[eqn] += vNE*v[c3+1];
            if (mesh->BCv.type[c3  ] != 11 && mesh->BCv.type[c3  ] != 0) StokesA->F[eqn] += vNW*v[c3];
        }
        //--------------------
        if ( mesh->BCp.type[c2+1] != 30  ) StokesA->F[eqn]  +=  pE*p[c2+1];
        StokesA->F[eqn] *= celvol;

        //        printf("%2.4f %2.4f\n", u[c1-nx], uS);
        //        printf("%2.4f %2.4f\n", u[c1], uC);
        //        printf("%2.4f %2.4f\n", u[c1+1],uE);
        ////        printf("%2.2e\n", u[c1+nx]);
        ////        printf("%2.2e\n", v[c3-nxvz]);
        //        printf("%2.4f %2.4f\n", v[c3-nxvz+1], vSE);
        ////        printf("%2.2e\n", v[c3]);
        ////        printf("%2.2e\n", v[c3+1]);
        //        printf("%2.4f %2.4f\n", p[c2+1], pE);
        //        printf("F1 = %2.4f\n", u[c1-nx]*uS+u[c1]*uC+u[c1+1]*uE+p[c2+1]*pE+v[c3-nxvz+1]*vSE);


        //        // Residual function
        //        StokesA->F[eqn] = uC*u[c1];
        //        if ( mesh->BCp.type[c2+1] != 30  ) {
        //            StokesA->F[eqn]  +=  pE*p[c2+1];
        //        }
        //        if ( mesh->BCv.type[c3-nxvz+1] == -1 || mesh->BCv.type[c3-nxvz+1] == 2 ) StokesA->F[eqn] += vSE*v[c3-nxvz+1];
        //        if ( mesh->BCv.type[c3-nxvz]   == -1 || mesh->BCv.type[c3-nxvz]   == 2 ) StokesA->F[eqn] += vSW*v[c3-nxvz];
        //        if ( mesh->BCv.type[c3+1]      == -1 || mesh->BCv.type[c3+1]      == 2 ) StokesA->F[eqn] += vNE*v[c3+1];
        //        if ( mesh->BCv.type[c3]        == -1 || mesh->BCv.type[c3]        == 2 ) StokesA->F[eqn] += vNW*v[c3];
        //
        //        if ( mesh->BCu.type[c1-nx] != 30 && mesh->BCu.type[c1-nx] != 11  ) StokesA->F[eqn] += uS*u[c1-nx];
        //        if ( mesh->BCu.type[c1+nx] != 30 && mesh->BCu.type[c1+nx] != 11  ) StokesA->F[eqn] += uN*u[c1+nx];
        //        if ( mesh->BCu.type[c1+1]  != 30 ) StokesA->F[eqn] += uE*u[c1+1];
        //        if ( mesh->BCu.type[c1-nx] == 11 ) StokesA->F[eqn] += -2*etaS*one_dz_dz*mesh->BCu.val[c1-nx];
        //        if ( mesh->BCu.type[c1+nx] == 11 ) StokesA->F[eqn] += -2*etaN*one_dz_dz*mesh->BCu.val[c1+nx];
        //
        //        StokesA->F[eqn] -= (StokesA->b[eqn]) + StokesA->bbc[eqn];
        //        StokesA->F[eqn] *= celvol;


        //        printf("\n F = %2.4e %2.2e %2.2e %2.2e\n\n", StokesA->F[eqn], StokesA->b[eqn], StokesA->bbc[eqn],  StokesA->b[eqn] + StokesA->bbc[eqn]);
        //        printf("%2.4e %2.4e  %2.4e %2.4e %2.4e\n", u[c1-nx], u[c1], u[c1+1], v[c3-nxvz+1], p[c2+1] );
        //        printf("F = %2.2e\n", (StokesA->b[eqn]+ StokesA->bbc[eqn] ) - (u[c1-nx]*uS + u[c1]*uC + u[c1+1]*uE + v[c3-nxvz+1]*vSE + p[c2+1]*pE));
    }

    ////    if (mesh->BCu.type[c1+nx]==30) {
    //    printf("x momentum W --  etaE = %2.2e etaS = %2.2e etaN = %2.2e\n", etaE, etaS, etaN);
    //    printf("uC = %02d %2.2e\n",      mesh->BCu.type[c1+0],  uC );
    //    //                    printf("uW = %02d %2.2e\n",      mesh->BCu.type[c1-1],  uW );
    //    printf("uE = %02d %2.2e\n",      mesh->BCu.type[c1+1],  uE );
    //    printf("uS = %02d %2.2e \n",     mesh->BCu.type[c1-nx], uS );
    //    printf("uN = %02d %2.2e \n",     mesh->BCu.type[c1+nx], uN );
    //    printf("vSE= %02d %2.2e \n",     mesh->BCv.type[c3-nxvz+1], vSE );
    //    printf("vNE= %02d %2.2e \n",     mesh->BCv.type[c3+1], vNE );
    //    printf("vSW= %02d %2.2e \n",     mesh->BCv.type[c3-nxvz], vSW );
    //    printf("vNW= %02d %2.2e \n",     mesh->BCv.type[c3], vNW );
    ////    //                    printf("pW = %02d %2.2e \n",     mesh->BCp.type[c2], pW );
    //    printf("pE = %02d %2.2e \n",mesh->BCp.type[c2+1], pE);
    ////    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void Xmomentum_EastNeumannDecoupled( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B  ) {

    double etaW, etaN, etaS, etaE;
    etaW = mesh->eta_n[c2];
    etaE = etaW;
    etaN = mesh->eta_s[c1];
    etaS = mesh->eta_s[c1-nx];

    //    double rhoVx = 0.5*(mesh->rho_s[c1] + mesh->rho_s[c1-nx]);
    double uS=0.0, uN=0.0, uW=0.0, uE=0.0, uC=0.0, vSW=0.0, vSE=0.0, vNW=0.0, vNE=0.0, pE=0.0, pW=0.0;

    StokesA->b[eqn] += mesh->BCu.val[c1]*one_dx;

    // dsxx/dx
    if ( mesh->BCu.type[c1-1] != 30  ) {
        uW  =  2.0*one_dx_dx * etaW         - comp*2.0/3.0*one_dx_dx * etaW;
        uC  = -2.0*one_dx_dx * (etaW )      + comp*2.0/3.0*one_dx_dx * etaW; //!!!!!!!!
        //        uE  =  2*one_dx_dx * etaE         - comp*2.0/3.0*one_dx_dx * etaE;
    }

    // eta*dvx/dz
    if ( mesh->BCu.type[c1-nx] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
        if ( mesh->BCu.type[c1-nx] != 13 ) {uS   =  one_dz_dz * etaS; uC  +=  -one_dz_dz * etaS;}
        if ( mesh->BCu.type[c1+nx] != 13 ) {uN   =  one_dz_dz * etaN; uC  +=  -one_dz_dz * etaN;}
    }
    if ( mesh->BCu.type[c1-nx] == 30 && mesh->BCu.type[c1+nx] != 30 ) {
        if ( mesh->BCu.type[c1+nx] != 13 ) uC  +=  -one_dz_dz * etaN;
        if ( mesh->BCu.type[c1+nx] != 13 ) uN   =   one_dz_dz * etaN;
    }
    if ( mesh->BCu.type[c1-nx] != 30 && mesh->BCu.type[c1+nx] == 30 ) {
        if ( mesh->BCu.type[c1-nx] != 13 ) uC  +=  -one_dz_dz * etaS;
        if ( mesh->BCu.type[c1-nx] != 13 ) uS   =   one_dz_dz * etaS;
    }

    if ( mesh->BCu.type[c1-nx] == 11 ) uC  +=  -one_dz_dz * etaS;
    if ( mesh->BCu.type[c1+nx] == 11 ) uC  +=  -one_dz_dz * etaN;

    // eta*dvz/dx
    if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3-nxvz+1] != 30 )  {
        vSE = -one_dx_dz * etaS + comp*2.0/3.0*one_dx_dz * etaE;
        vSW =  one_dx_dz * etaS - comp*2.0/3.0*one_dx_dz * etaW;
    }

    if ( mesh->BCv.type[c3+1] != 30 && mesh->BCv.type[c3] != 30  ) {
        vNE =  one_dx_dz * etaN - comp*2.0/3.0*one_dx_dz * etaE;
        vNW = -one_dx_dz * etaN + comp*2.0/3.0*one_dx_dz * etaW;
    }
    // Pressure gradient
    if ( mesh->BCp.type[c2] != 30) {
        pW  =   one_dx;
    }

    //    // Inertia
    //    if ( model.isinertial == 1 || model.isinertial == 2 ) {
    //        uC -= sign * rhoVx/model.dt;
    //    }
    //    if ( model.isinertial == 2 ) {
    //        uN -= rhoVx/2*one_dz*mesh->VzVx[c1];
    //        uS += rhoVx/2*one_dz*mesh->VzVx[c1];
    //        //        uW += rhoVx/2*one_dx*mesh->u_in[c1];
    //        uE -= rhoVx/2*one_dx*mesh->u_in[c1];
    //    }

    // Stabilisation with density gradients
    if ( stab == 1 ) {
        //        double correction = - om*0.5*model.dt * model.gx * (mesh->rho_n[c2+1] - mesh->rho_n[c2]) * one_dx;
        //        uC += correction;
        //        correction = - om*0.5*model.dt * model.gx * (mesh->rho_s[c1] - mesh->rho_s[c1-nx]) * one_dz;
        //        vSW += 0.25*correction;
        //        vNE += 0.25*correction;
        //        vNW += 0.25*correction;
        //        vSE += 0.25*correction;
    }

    uS=-uS, uN=-uN, uW=-uW, uE=-uE, uC=-uC, vSW=-vSW, vSE=-vSE, vNW=-vNW, vNE=-vNE, pE=-pE, pW=-pW;

    if ( Assemble == 1 ) {

        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------
        // dsxx/dx - normal stencil
        if (mesh->BCu.type[c1-nx] != 30) {
            if (mesh->BCu.type[c1-nx] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-nx], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[c1-nx], mesh->BCu.val[c1-nx], StokesA->bbc);
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-nx], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[c1-nx], 2.0*mesh->BCu.val[c1-nx], StokesA->bbc);
        }
        if (mesh->BCu.type[c1-1]  != 30)     AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-1],  &(nnzc2A[ith]), uW*celvol, mesh->BCu.type[c1-1],  mesh->BCu.val[c1-1],  StokesA->bbc);
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                  &(nnzc2A[ith]), uC*celvol, mesh->BCu.type[c1],    mesh->BCu.val[c1],    StokesA->bbc);
        if (mesh->BCu.type[c1+nx] != 30) {
            if (mesh->BCu.type[c1+nx] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[c1+nx], mesh->BCu.val[c1+nx], StokesA->bbc);
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[c1+nx], 2.0*mesh->BCu.val[c1+nx], StokesA->bbc);
        }
        //--------------------

        if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3-nxvz+1] != 30 )  {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz],   &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[c3-nxvz],   mesh->BCv.val[c3-nxvz],   StokesA->bbc);
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz+1], &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[c3-nxvz+1], mesh->BCv.val[c3-nxvz+1], StokesA->bbc);
        }

        if ( mesh->BCv.type[c3] != 30 && mesh->BCv.type[c3+1] != 30 )  {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3],        &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[c3],        mesh->BCv.val[c3],        StokesA->bbc);
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],      &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[c3+1],      mesh->BCv.val[c3+1],      StokesA->bbc);
        }

        //--------------------
        if ( mesh->BCp.type[c2] != 30  ) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2] - Stokes->neq_mom,   &(nnzc2B[ith]), pW*celvol, mesh->BCp.type[c2],   mesh->BCp.val[c2],   StokesB->bbc);
        }
    }
    else {
        // RHS
        StokesA->F[eqn] = -(StokesA->b[eqn] + StokesA->bbc[eqn]/celvol);
        StokesA->F[eqn] += uC*u[c1];
        if (mesh->BCu.type[c1-nx] != 30) {
            if (mesh->BCu.type[c1-nx] != 11) StokesA->F[eqn] += uS*u[c1-nx];
        }
        if (mesh->BCu.type[c1-1]  != 30)     StokesA->F[eqn] += uW*u[c1-1];
        if (mesh->BCu.type[c1+nx] != 30) {
            if (mesh->BCu.type[c1+nx] != 11) StokesA->F[eqn] += uN*u[c1+nx];
        }
        //--------------------
        if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3-nxvz+1] != 30 )  {
            if (mesh->BCv.type[c3-nxvz+1] != 11 && mesh->BCv.type[c3-nxvz+1] != 0) StokesA->F[eqn] += vSE*v[c3-nxvz+1];
            if (mesh->BCv.type[c3-nxvz  ] != 11 && mesh->BCv.type[c3-nxvz  ] != 0) StokesA->F[eqn] += vSW*v[c3-nxvz];
        }
        if ( mesh->BCv.type[c3] != 30 && mesh->BCv.type[c3+1] != 30 )  {
            if (mesh->BCv.type[c3+1] != 11 && mesh->BCv.type[c3+1] != 0) StokesA->F[eqn] += vNE*v[c3+1];
            if (mesh->BCv.type[c3  ] != 11 && mesh->BCv.type[c3  ] != 0) StokesA->F[eqn] += vNW*v[c3];
        }
        //--------------------
        if ( mesh->BCp.type[c2] != 30  ) StokesA->F[eqn]  +=  pW*p[c2];
        StokesA->F[eqn] *= celvol;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void Zmomentum_InnerNodesDecoupled( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B ) {

    double uSW=0.0, uSE=0.0, uNW=0.0, uNE=0.0, vS=0.0, vW=0.0, vC=0.0, vE=0.0, vN=0.0, pN=0.0, pS=0.0;

    // Coefficients
    double D22S  = mesh->D22_n[c2];
    double D22N  = mesh->D22_n[c2+ncx];
    double D33E  = mesh->D33_s[c1];
    double D33W  = mesh->D33_s[c1-1];

    if (mesh->BCp.type[c2+ncx] == 31)     {
        StokesA->b[eqn] += mesh->BCv.val[c3]*one_dz;
    }

    double inS=0.0,inN=0.0,inWu=0.0,inWv=0.0,inEu=0.0,inEv=0.0;

    // dsyy/dz
    if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3+nxvz] !=30 ) {
        inS = 1.0;
        inN = 1.0;
    }
    if ( mesh->BCv.type[c3-nxvz] == 30 && mesh->BCv.type[c3+nxvz] != 30 ) {
        inN = 1.0;
    }
    if ( mesh->BCv.type[c3-nxvz] != 30  && mesh->BCv.type[c3+nxvz] == 30 ) {
        inS = 1.0;
    }

    if (mesh->BCp.type[c2    ] == -1) inS = 1.0;
    if (mesh->BCp.type[c2+ncx] == -1) inN = 1.0;

    double inW=0.0, inE = 0.0;
    if (mesh->BCg.type[c1-1] != 30 && mesh->BCv.type[c3-1] != 13 ) inW = 1.0;
    if (mesh->BCg.type[c1  ] != 30 && mesh->BCv.type[c3+1] != 13 ) inE = 1.0;

    double dx = mesh->dx;
    double dz = mesh->dz;

    comp = 1.0;
    // simpler
    vW = -D33W*inW/pow(dx, 2);
    vC = -(D22N*inN*((1.0/3.0)*comp/dz - 1/dz) - D22S*inS*(-1.0/3.0*comp/dz + 1.0/dz))/dz - (-D33E*inE/dx - D33W*inW/dx)/dx;
    vE = -D33E*inE/pow(dx, 2);
    vS =  D22S*inS*((1.0/3.0)*comp/dz - 1/dz)/dz;
    vN = -D22N*inN*(-1.0/3.0*comp/dz + 1.0/dz)/dz;
    uSW = (1.0/3.0)*D22S*comp*inS/(dx*dz) - D33W*inW/(dx*dz);
    uSE = -1.0/3.0*D22S*comp*inS/(dx*dz) + D33E*inE/(dx*dz);
    uNW = -1.0/3.0*D22N*comp*inN/(dx*dz) + D33W*inW/(dx*dz);
    uNE = (1.0/3.0)*D22N*comp*inN/(dx*dz) - D33E*inE/(dx*dz);

    if ( mesh->BCv.type[c3-1] == 11 ) vC  -=  vW;
    if ( mesh->BCv.type[c3+1] == 11 ) vC  -=  vE;


    // Stabilisation with density gradients
    if (stab==1) {
        double drhodz  = (mesh->rho_n[c2+ncx] - mesh->rho_n[c2])*one_dz;
        double drhodx  = (mesh->rho_s[c1-1  ] - mesh->rho_s[c1])*one_dx;
        double vC_corr = 1.00 * om * model.dt * mesh->gz[c3] * drhodz;
        double vy_corr = 0.25 * om * model.dt * mesh->gz[c3] * drhodx;
        
//        if (mesh->rho_n[c2+ncx]*1e27<1000) {
//            drhodz = 3*drhodz;
////            printf("mesh->rho_n[c2+ncx] = %2.2e\n", mesh->rho_n[c2+ncx]*1e27);
////            printf("mesh->rho_n[c2] = %2.2e\n", mesh->rho_n[c2]*1e27);
//        }
        // Importante trique, voire meme gigantesque!
        if (vC+vC_corr>0.0) {
            vC  += vC_corr;
//            uSW += vy_corr;
//            uSE += vy_corr;
//            uNW += vy_corr;
//            uNE += vy_corr;
//            if (vC_corr>1e-6)printf("vC_corr = %2.2e gz=%2.2e\n",vC_corr, mesh->gz[c3]);
//            printf(mesh->rho_n[c2+ncx])
                  
        }
    }

    // Pressure gradient
    if ( mesh->BCp.type[c2] != 30 && mesh->BCp.type[c2+ncx] != 30 ) {
        pS  =  -one_dz * 1e0;
        pN  =   one_dz * 1e0;
    }

//    uSW=-uSW, uSE=-uSE, uNW=-uNW, uNE=-uNE, vS=-vS, vW=-vW, vC=-vC, vE=-vE, vN=-vN, pN=-pN, pS=-pS;

    if ( Assemble == 1 ) {
        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------
//        if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-1],     &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[c1-1],    mesh->BCu.val[c1-1],    StokesA->bbc );
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1],       &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[c1],      mesh->BCu.val[c1],      StokesA->bbc );
//        }
//        if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30) {
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx-1],  &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[c1+nx-1], mesh->BCu.val[c1+nx-1], StokesA->bbc );
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx],    &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[c1+nx],   mesh->BCu.val[c1+nx],   StokesA->bbc );
//        }

//                if ( mesh->BCu.type[c1+nx] != 30 && mesh->BCu.type[c1] != 30 ) {
//                    AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx],    &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[c1+nx],   mesh->BCu.val[c1+nx],   StokesA->bbc );
//                    AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1],       &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[c1],      mesh->BCu.val[c1],      StokesA->bbc );
//                }
//                if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1-1] != 30) {
//                    AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx-1],  &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[c1+nx-1], mesh->BCu.val[c1+nx-1], StokesA->bbc );
//                    AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-1],     &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[c1-1],    mesh->BCu.val[c1-1],    StokesA->bbc );
//                }
//

        if ( mesh->BCu.type[c1+nx]   != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx],    &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[c1+nx],   mesh->BCu.val[c1+nx],   StokesA->bbc );
        if ( mesh->BCu.type[c1]      != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1],       &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[c1],      mesh->BCu.val[c1],      StokesA->bbc );
        if ( mesh->BCu.type[c1+nx-1] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx-1],  &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[c1+nx-1], mesh->BCu.val[c1+nx-1], StokesA->bbc );
        if ( mesh->BCu.type[c1-1]    != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-1],     &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[c1-1],    mesh->BCu.val[c1-1],    StokesA->bbc );

        //--------------------
        if ( mesh->BCv.type[c3-nxvz] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz],  &(nnzc2A[ith]), vS*celvol, mesh->BCv.type[c3-nxvz], mesh->BCv.val[c3-nxvz], StokesA->bbc );
        if ( mesh->BCv.type[c3-1] != 30 && mesh->BCv.type[c3-1] != -12  ) {
            if( mesh->BCv.type[c3-1] != 11 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-1],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3-1],    mesh->BCv.val[c3-1],    StokesA->bbc );
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-1],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3-1],    2.0*mesh->BCv.val[c3-1],    StokesA->bbc );
        }
        // Periodic
        if ( mesh->BCv.type[c3-1] == -12 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+nxvz-3],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3+nxvz-3],    mesh->BCv.val[c3+nxvz-3],    StokesA->bbc );
        }
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                     &(nnzc2A[ith]), vC*celvol, mesh->BCv.type[c3],      mesh->BCv.val[c3],      StokesA->bbc );
        if ( mesh->BCv.type[c3+1] != 30 && mesh->BCv.type[c3+1] != -12  ) {
            if ( mesh->BCv.type[c3+1] != 11 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3+1],    mesh->BCv.val[c3+1],    StokesA->bbc );
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3+1],    2.0*mesh->BCv.val[c3+1],    StokesA->bbc );
        }
        // Periodic
        if ( mesh->BCv.type[c3+1] == -12 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz+3],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3-nxvz+3],    mesh->BCv.val[c3-nxvz+3],    StokesA->bbc );
        }

        if ( mesh->BCv.type[c3+nxvz] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+nxvz],  &(nnzc2A[ith]), vN*celvol, mesh->BCv.type[c3+nxvz], mesh->BCv.val[c3+nxvz], StokesA->bbc );
        //--------------------
        if ( mesh->BCp.type[c2] != 30 && mesh->BCp.type[c2+ncx] != 30) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2] - Stokes->neq_mom,      &(nnzc2B[ith]), pS*celvol, mesh->BCp.type[c2],     mesh->BCp.val[c2],     StokesB->bbc );
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2+ncx] - Stokes->neq_mom,  &(nnzc2B[ith]), pN*celvol, mesh->BCp.type[c2+ncx], mesh->BCp.val[c2+ncx], StokesB->bbc );
        }

    }
    else {

        // Residual
        StokesA->F[eqn] = vC*v[c3];
        if ( mesh->BCp.type[c2] != 30 && mesh->BCp.type[c2+ncx] != 30 ) {
            StokesA->F[eqn]  += pS*p[c2] + pN*p[c2+ncx];
        }
        if ( mesh->BCv.type[c3-nxvz] != 30 ) StokesA->F[eqn] += vS*v[c3-nxvz];
        if ( mesh->BCv.type[c3-1]    != 30 && mesh->BCv.type[c3-1] != 11 && mesh->BCv.type[c3-1] != -12 ) StokesA->F[eqn] += vW*v[c3-1];
        if ( mesh->BCv.type[c3+1]    != 30 && mesh->BCv.type[c3+1] != 11 && mesh->BCv.type[c3+1] != -12 ) StokesA->F[eqn] += vE*v[c3+1];
        if ( mesh->BCv.type[c3+nxvz] != 30 ) StokesA->F[eqn] += vN*v[c3+nxvz];
//        if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
//            StokesA->F[eqn] += uSW*u[c1-1] + uSE*u[c1];
//        }
//        if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
//            StokesA->F[eqn] += uNW*u[c1+nx-1] + uNE*u[c1+nx];
//        }

        if ( mesh->BCu.type[c1-1] != 30 )  StokesA->F[eqn] += uSW*u[c1-1];
        if ( mesh->BCu.type[c1]   != 30 )  StokesA->F[eqn] += uSE*u[c1];

        if ( mesh->BCu.type[c1+nx-1] != 30 ) StokesA->F[eqn] += uNW*u[c1+nx-1];
        if ( mesh->BCu.type[c1+nx]   != 30 ) StokesA->F[eqn] += uNE*u[c1+nx];


        if ( mesh->BCv.type[c3-1] == 11 )   StokesA->F[eqn] += -2*D33W*one_dx_dx*mesh->BCv.val[c3-1];
        if ( mesh->BCv.type[c3+1] == 11 )   StokesA->F[eqn] += -2*D33E*one_dx_dx*mesh->BCv.val[c3+1];
        if ( mesh->BCv.type[c3-1] == -12 ) StokesA->F[eqn] += vW*v[c3+nxvz-3];
        if ( mesh->BCv.type[c3+1] == -12 ) StokesA->F[eqn] += vE*v[c3-nxvz+3];
        StokesA->F[eqn] -= (StokesA->b[eqn]);
        StokesA->F[eqn] *= celvol;
//        printf("StokesA->F[eqn] = %2.2e %2.2e %2.2e %2.2e %2.2e\n", StokesA->F[eqn], vC*v[c3], vS*v[c3-nxvz], vW*v[c3-1], StokesA->b[eqn]);
    }
}


/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

//void Zmomentum_InnerNodesDecoupled_v4.5( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B ) {
//
//    double uSW=0.0, uSE=0.0, uNW=0.0, uNE=0.0, vS=0.0, vW=0.0, vC=0.0, vE=0.0, vN=0.0, pN=0.0, pS=0.0;
//
//    // Coefficients
//    double AS  = mesh->eta_n[c2];
//    double AN  = mesh->eta_n[c2+ncx];
//    double AE  = mesh->eta_s[c1];
//    double AW  = mesh->eta_s[c1-1];
//
//
//    //    if(eqn==13400) {
//    //        printf("%d AW = %2.6e AE = %2.6e\n", eqn, AW, AE);
//    //               printf("%d AS = %2.6e AN = %2.6e\n", eqn, AS, AN);
//    //    }
//
//
//    if (mesh->BCp.type[c2+ncx] == 31)     {
//        StokesA->b[eqn] += mesh->BCv.val[c3]*one_dz;
//        //        printf("Syy = %2.2e\n", mesh->BCv.val[c3]);
//    }
//
//
//    // (eta du/dx) S
//    if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
//        uSW =  one_dx_dz * AW - comp*2.0/3.0*AS*one_dx_dz;
//        uSE = -one_dx_dz * AE + comp*2.0/3.0*AS*one_dx_dz;
//    }
//
//    // (eta du/dx) N
//    if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
//        uNW = -one_dx_dz * AW + comp*2.0/3.0*AN*one_dx_dz;
//        uNE =  one_dx_dz * AE - comp*2.0/3.0*AN*one_dx_dz;
//    }
//
//    //    // (eta du/dx) S
//    //    if ( mesh->BCu.type[c1+nx] != 30 && mesh->BCu.type[c1] != 30 ) {
//    //        uNE =  one_dx_dz * AE - comp*2.0/3.0*AN*one_dx_dz;
//    //        uSE = -one_dx_dz * AE + comp*2.0/3.0*AS*one_dx_dz;
//    //    }
//    //
//    //    // (eta du/dx) N
//    //    if ( mesh->BCu.type[c1+nx-1] != 30 &&  mesh->BCu.type[c1-1] != 30 ) {
//    //        uNW = -one_dx_dz * AW + comp*2.0/3.0*AN*one_dx_dz;
//    //        uSW =  one_dx_dz * AW - comp*2.0/3.0*AS*one_dx_dz;
//    //    }
//
//
//    // dsyy/dz
//    if ( mesh->BCv.type[c3-nxvz] != 30 && mesh->BCv.type[c3+nxvz] !=30 ) {
//        vS  =  2.0*one_dz_dz * AS;//        - comp*2.0/3.0*AS*one_dz_dz;
//        vC  = -2.0*one_dz_dz * (AN + AS);// + comp*2.0/3.0*AN*one_dz_dz + comp*2.0/3.0*AS*one_dz_dz;
//        vN  =  2.0*one_dz_dz * AN;//        - comp*2.0/3.0*AN*one_dz_dz;
//    }
//    if ( mesh->BCv.type[c3-nxvz] == 30 && mesh->BCv.type[c3+nxvz] != 30 ) {
//        vC  = -2.0*one_dz_dz * (AN) + comp*2.0/3.0*AN*one_dz_dz;
//        vN  =  2.0*one_dz_dz * AN   - comp*2.0/3.0*AN*one_dz_dz;
//    }
//    if ( mesh->BCv.type[c3-nxvz] != 30  && mesh->BCv.type[c3+nxvz] == 30 ) {
//        vS  =  2.0*one_dz_dz * AS   - comp*2.0/3.0*AS*one_dz_dz;
//        vC  = -2.0*one_dz_dz * (AS) + comp*2.0/3.0*AS*one_dz_dz;
//    }
//
//    // d/dx eta*dvzdx
//    if ( mesh->BCv.type[c3-1] != 30 && mesh->BCv.type[c3+1] != 30 ) {
//        if ( mesh->BCv.type[c3-1] != 13 ) {vW  =  one_dx_dx * AW; vC += -one_dx_dx * AW;}
//        if ( mesh->BCv.type[c3+1] != 13 ) {vE  =  one_dx_dx * AE; vC += -one_dx_dx * AE;}
//    }
//    // d/dx eta*dvzdx
//    if ( mesh->BCv.type[c3-1] == 30 && mesh->BCv.type[c3+1] != 30 ) {
//        if ( mesh->BCv.type[c3+1] != 13 ) vE  =  one_dx_dx * AE;
//        if ( mesh->BCv.type[c3+1] != 13 ) vC += -one_dx_dx * AE;
//    }
//    // d/dx eta*dvzdx
//    if ( mesh->BCv.type[c3-1] != 30 && mesh->BCv.type[c3+1] == 30 ) {
//        if ( mesh->BCv.type[c3-1] != 13 ) vW  =  one_dx_dx * AW;
//        if ( mesh->BCv.type[c3-1] != 13 ) vC += -one_dx_dx * AW;
//    }
//
//    if ( mesh->BCv.type[c3-1] == 11 ) vC  +=  -one_dx_dx * AW;
//    if ( mesh->BCv.type[c3+1] == 11 ) vC  +=  -one_dx_dx * AE;
//
//    // Stabilisation with density gradients
//    if (stab==1) {
//        double drhodz  = (mesh->rho_n[c2+ncx] - mesh->rho_n[c2])*one_dz;
//        double vC_corr = - 1.00 * om * model.dt * model.gz * drhodz;
//        // Importante trique, voire meme gigantesque!
//
//        if (vC+vC_corr<0.0)  vC += vC_corr;
//        //        if (vW+.25*vC_corr>0)  vW += .25*vC_corr;
//        //        if (vE+.25*vC_corr>0)  vE += .25*vC_corr;
//        //        if (vS+.25*vC_corr>0)  vS += .25*vC_corr;
//        //        if (vN+.25*vC_corr>0)  vN += .25*vC_corr;
//
//        // Non-symmetric contibution to the system of equation
//        //        double drhodx = (mesh->rho_s[c1]   - mesh->rho_s[c1-1])*one_dx;
//        //        uNW += - 0.25 * om * model.dt * model.gz * drhodx;
//        //        uNE += - 0.25 * om * model.dt * model.gz * drhodx;
//        //        uSW += - 0.25 * om * model.dt * model.gz * drhodx;
//        //        uSE += - 0.25 * om * model.dt * model.gz * drhodx;
//    }
//
//    // Pressure gradient
//    if ( mesh->BCp.type[c2] != 30 && mesh->BCp.type[c2+ncx] != 30 ) {
//        pS  =   one_dz;
//        pN  =  -one_dz;
//    }
//
//    uSW=-uSW, uSE=-uSE, uNW=-uNW, uNE=-uNE, vS=-vS, vW=-vW, vC=-vC, vE=-vE, vN=-vN, pN=-pN, pS=-pS;
//
//    if ( Assemble == 1 ) {
//        StokesA->b[eqn] *= celvol;
//        StokesB->b[eqn] *= celvol;
//        //--------------------
//        if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-1],     &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[c1-1],    mesh->BCu.val[c1-1],    StokesA->bbc );
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1],       &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[c1],      mesh->BCu.val[c1],      StokesA->bbc );
//        }
//        if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30) {
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx-1],  &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[c1+nx-1], mesh->BCu.val[c1+nx-1], StokesA->bbc );
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx],    &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[c1+nx],   mesh->BCu.val[c1+nx],   StokesA->bbc );
//        }
//
//        //        if ( mesh->BCu.type[c1+nx] != 30 && mesh->BCu.type[c1] != 30 ) {
//        //            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx],    &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[c1+nx],   mesh->BCu.val[c1+nx],   StokesA->bbc );
//        //            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1],       &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[c1],      mesh->BCu.val[c1],      StokesA->bbc );
//        //        }
//        //        if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1-1] != 30) {
//        //            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx-1],  &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[c1+nx-1], mesh->BCu.val[c1+nx-1], StokesA->bbc );
//        //            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-1],     &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[c1-1],    mesh->BCu.val[c1-1],    StokesA->bbc );
//        //        }
//
//        //--------------------
//        if ( mesh->BCv.type[c3-nxvz] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz],  &(nnzc2A[ith]), vS*celvol, mesh->BCv.type[c3-nxvz], mesh->BCv.val[c3-nxvz], StokesA->bbc );
//        if ( mesh->BCv.type[c3-1] != 30 && mesh->BCv.type[c3-1] != -12  ) {
//            if( mesh->BCv.type[c3-1] != 11 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-1],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3-1],    mesh->BCv.val[c3-1],    StokesA->bbc );
//            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-1],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3-1],    2.0*mesh->BCv.val[c3-1],    StokesA->bbc );
//        }
//        // Periodic
//        if ( mesh->BCv.type[c3-1] == -12 ) {
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+nxvz-3],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3+nxvz-3],    mesh->BCv.val[c3+nxvz-3],    StokesA->bbc );
//        }
//        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                     &(nnzc2A[ith]), vC*celvol, mesh->BCv.type[c3],      mesh->BCv.val[c3],      StokesA->bbc );
//        if ( mesh->BCv.type[c3+1] != 30 && mesh->BCv.type[c3+1] != -12  ) {
//            if ( mesh->BCv.type[c3+1] != 11 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3+1],    mesh->BCv.val[c3+1],    StokesA->bbc );
//            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3+1],    2.0*mesh->BCv.val[c3+1],    StokesA->bbc );
//        }
//        // Periodic
//        if ( mesh->BCv.type[c3+1] == -12 ) {
//            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz+3],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3-nxvz+3],    mesh->BCv.val[c3-nxvz+3],    StokesA->bbc );
//        }
//
//        if ( mesh->BCv.type[c3+nxvz] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+nxvz],  &(nnzc2A[ith]), vN*celvol, mesh->BCv.type[c3+nxvz], mesh->BCv.val[c3+nxvz], StokesA->bbc );
//        //--------------------
//        if ( mesh->BCp.type[c2] != 30 && mesh->BCp.type[c2+ncx] != 30) {
//            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2] - Stokes->neq_mom,      &(nnzc2B[ith]), pS*celvol, mesh->BCp.type[c2],     mesh->BCp.val[c2],     StokesB->bbc );
//            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2+ncx] - Stokes->neq_mom,  &(nnzc2B[ith]), pN*celvol, mesh->BCp.type[c2+ncx], mesh->BCp.val[c2+ncx], StokesB->bbc );
//        }
//
//    }
//    else {
//
//        // Residual
//        StokesA->F[eqn] = vC*v[c3];
//        if ( mesh->BCp.type[c2] != 30 && mesh->BCp.type[c2+ncx] != 30 ) {
//            StokesA->F[eqn]  += pS*p[c2] + pN*p[c2+ncx];
//        }
//        if ( mesh->BCv.type[c3-nxvz] != 30 ) StokesA->F[eqn] += vS*v[c3-nxvz];
//        if ( mesh->BCv.type[c3-1]    != 30 && mesh->BCv.type[c3-1] != 11 && mesh->BCv.type[c3-1] != -12 ) StokesA->F[eqn] += vW*v[c3-1];
//        if ( mesh->BCv.type[c3+1]    != 30 && mesh->BCv.type[c3+1] != 11 && mesh->BCv.type[c3+1] != -12 ) StokesA->F[eqn] += vE*v[c3+1];
//        if ( mesh->BCv.type[c3+nxvz] != 30 ) StokesA->F[eqn] += vN*v[c3+nxvz];
//        if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
//            StokesA->F[eqn] += uSW*u[c1-1] + uSE*u[c1];
//        }
//        if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
//            StokesA->F[eqn] += uNW*u[c1+nx-1] + uNE*u[c1+nx];
//        }
//        if ( mesh->BCv.type[c3-1] == 11 )   StokesA->F[eqn] += -2*AW*one_dx_dx*mesh->BCv.val[c3-1];
//        if ( mesh->BCv.type[c3+1] == 11 )   StokesA->F[eqn] += -2*AE*one_dx_dx*mesh->BCv.val[c3+1];
//        if ( mesh->BCv.type[c3-1] == -12 ) StokesA->F[eqn] += vW*v[c3+nxvz-3];
//        if ( mesh->BCv.type[c3+1] == -12 ) StokesA->F[eqn] += vE*v[c3-nxvz+3];
//        StokesA->F[eqn] -= (StokesA->b[eqn]);
//        StokesA->F[eqn] *= celvol;
//    }
//}
/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void Zmomentum_SouthNeumannDecoupled( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B ) {

    double uSW=0.0, uSE=0.0, uNW=0.0, uNE=0.0, vS=0.0, vW=0.0, vC=0.0, vE=0.0, vN=0.0, pN=0.0, pS=0.0;

    // Coefficients
    double etaS, etaN, etaW, etaE;
    etaN  = mesh->eta_n[c2+ncx];
    etaS  = etaN;
    etaE  = mesh->eta_s[c1];
    etaW  = mesh->eta_s[c1-1];

    StokesA->b[eqn] -= mesh->BCv.val[c3]*one_dz;

    // (eta du/dx) S
    if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
        uSW =  one_dx_dz * etaW - comp*2.0/3.0*etaS*one_dx_dz;
        uSE = -one_dx_dz * etaE + comp*2.0/3.0*etaS*one_dx_dz;
    }

    // (eta du/dx) N
    if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
        uNW = -one_dx_dz * etaW + comp*2.0/3.0*etaN*one_dx_dz;
        uNE =  one_dx_dz * etaE - comp*2.0/3.0*etaN*one_dx_dz;
    }

    // dsyy/dz
    if (  mesh->BCv.type[c3+nxvz] !=30 ) { //mesh->BCv.type[c3-nxvz] != 30 &&
        vC  = -2.0*one_dz_dz * ( etaN ) + comp*2.0/3.0*etaS*one_dz_dz;
        vN  =  2.0*one_dz_dz *   etaN   - comp*2.0/3.0*etaN*one_dz_dz;
    }

    // d/dx eta*dvzdx
    if ( mesh->BCv.type[c3-1] != 30 && mesh->BCv.type[c3+1] != 30 ) {
        if ( mesh->BCv.type[c3-1] != 13 ) {vW  =  one_dx_dx * etaW; vC += -one_dx_dx * etaW;}
        if ( mesh->BCv.type[c3+1] != 13 ) {vE  =  one_dx_dx * etaE; vC += -one_dx_dx * etaE;}

    }
    // d/dx eta*dvzdx
    if ( mesh->BCv.type[c3-1] == 30 && mesh->BCv.type[c3+1] != 30 ) {
        if ( mesh->BCv.type[c3+1] != 13 ) vE  =  one_dx_dx * etaE;
        if ( mesh->BCv.type[c3+1] != 13 ) vC += -one_dx_dx * etaE;
    }
    // d/dx eta*dvzdx
    if ( mesh->BCv.type[c3-1] != 30 && mesh->BCv.type[c3+1] == 30 ) {
        if ( mesh->BCv.type[c3-1] != 13 ) vW  =  one_dx_dx * etaW;
        if ( mesh->BCv.type[c3-1] != 13 ) vC += -one_dx_dx * etaW;
    }

    if ( mesh->BCv.type[c3-1] == 11 ) vC  +=  -one_dx_dx * etaW;
    if ( mesh->BCv.type[c3+1] == 11 ) vC  +=  -one_dx_dx * etaE;

    // Stabilisation with density gradients
    if (stab==1) {
        double drhodz = (mesh->rho_n[c2+ncx] - mesh->rho_n[c2])*one_dz;
        //        double drhodx = (mesh->rho_s[c1]   - mesh->rho_s[c1-1])*one_dx;
        vC  +=  1.00 * om * model.dt * model.gz * drhodz;
        //        // Non-symmetric contibution to the system of equation
        //        uNW += - 0.25 * om * model.dt * model.gz * drhodx;
        //        uNE += - 0.25 * om * model.dt * model.gz * drhodx;
        //        uSW += - 0.25 * om * model.dt * model.gz * drhodx;
        //        uSE += - 0.25 * om * model.dt * model.gz * drhodx;
    }

    // Pressure gradient
    if ( mesh->BCp.type[c2+ncx] != 30 ) {
        pN  =  -one_dz;
    }

    uSW=-uSW, uSE=-uSE, uNW=-uNW, uNE=-uNE, vS=-vS, vW=-vW, vC=-vC, vE=-vE, vN=-vN, pN=-pN, pS=-pS;

    //    printf("%1.1e %1.1e %1.1e %1.1e %1.1e %1.1e %1.1e %1.1e %1.1e\n", vC, vW,  vE, vN, uSW, uSE, uNW, uNE, pN);



    if ( Assemble == 1 ) {
        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------
        if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-1],     &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[c1-1],    mesh->BCu.val[c1-1],    StokesA->bbc );
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1],       &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[c1],      mesh->BCu.val[c1],      StokesA->bbc );
        }
        if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx-1],  &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[c1+nx-1], mesh->BCu.val[c1+nx-1], StokesA->bbc );
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx],    &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[c1+nx],   mesh->BCu.val[c1+nx],   StokesA->bbc );
        }
        //--------------------
        if ( mesh->BCv.type[c3-1] != 30 ) {
            if( mesh->BCv.type[c3-1] != 11 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-1],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3-1],    mesh->BCv.val[c3-1],    StokesA->bbc );
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-1],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3-1],    2*mesh->BCv.val[c3-1],    StokesA->bbc );
        }
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                     &(nnzc2A[ith]), vC*celvol, mesh->BCv.type[c3],      mesh->BCv.val[c3],      StokesA->bbc );
        if ( mesh->BCv.type[c3+1] != 30 ) {
            if ( mesh->BCv.type[c3+1] != 11 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3+1],    mesh->BCv.val[c3+1],    StokesA->bbc );
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3+1],    2*mesh->BCv.val[c3+1],    StokesA->bbc );
        }
        if ( mesh->BCv.type[c3+nxvz] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+nxvz],  &(nnzc2A[ith]), vN*celvol, mesh->BCv.type[c3+nxvz], mesh->BCv.val[c3+nxvz], StokesA->bbc );
        //--------------------
        if ( mesh->BCp.type[c2+ncx] != 30) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2+ncx] - Stokes->neq_mom,  &(nnzc2B[ith]), pN*celvol, mesh->BCp.type[c2+ncx], mesh->BCp.val[c2+ncx], StokesB->bbc );
        }
        //        printf("Vz South residual = %2.2e b= %2.2e sum.c=%2.2e\n", StokesA->F[eqn], StokesA->b[eqn], uSW+uSE+uNW+uNE+vS+vW+vC+vE+vN+pN+pS);

    }
    else {
        // RHS
        StokesA->F[eqn] = -(StokesA->b[eqn] + StokesA->bbc[eqn]/celvol);
        StokesA->F[eqn] += vC*v[c3];
        if (mesh->BCv.type[c3-1] != 30) {
            if (mesh->BCv.type[c3-1] != 11) StokesA->F[eqn] += vW*v[c3-1];
        }
        if (mesh->BCv.type[c3+nxvz]  != 30) StokesA->F[eqn] += vN*v[c3+nxvz];
        if (mesh->BCv.type[c3+1] != 30) {
            if (mesh->BCv.type[c3+1] != 11) StokesA->F[eqn] += vE*v[c3+1];
        }
        //--------------------
        if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
            if (mesh->BCu.type[c1-1] != 11 && mesh->BCu.type[c1-1] != 0) StokesA->F[eqn] += uSW*u[c1-1];
            if (mesh->BCu.type[c1  ] != 11 && mesh->BCu.type[c1  ] != 0) StokesA->F[eqn] += uSE*u[c1  ];
        }
        if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
            if (mesh->BCu.type[c1+nx-1] != 11 && mesh->BCu.type[c1+nx-1] != 0) StokesA->F[eqn] += uNW*u[c1+nx-1];
            if (mesh->BCu.type[c1+nx  ] != 11 && mesh->BCu.type[c1+nx  ] != 0) StokesA->F[eqn] += uNE*u[c1+nx];
        }
        //--------------------
        if ( mesh->BCp.type[c2+ncx] != 30  ) StokesA->F[eqn]  +=  pN*p[c2+ncx];
        StokesA->F[eqn] *= celvol;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void Zmomentum_NorthNeumannDecoupled( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B ) {

    double uSW=0.0, uSE=0.0, uNW=0.0, uNE=0.0, vS=0.0, vW=0.0, vC=0.0, vE=0.0, vN=0.0, pN=0.0, pS=0.0;

    // Coefficients
    double etaS, etaW, etaE, etaN;
    etaS  = mesh->eta_n[c2];
    etaE  = mesh->eta_s[c1];
    etaW  = mesh->eta_s[c1-1];
    etaN  = etaS;

    StokesA->b[eqn] += mesh->BCv.val[c3]*one_dz;

    // (eta du/dx) S
    if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
        uSW =  one_dx_dz * etaW - comp*2.0/3.0*etaS*one_dx_dz;
        uSE = -one_dx_dz * etaE + comp*2.0/3.0*etaS*one_dx_dz;
    }

    // (eta du/dx) N
    if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
        uNW = -one_dx_dz * etaW + comp*2.0/3.0*etaN*one_dx_dz;
        uNE =  one_dx_dz * etaE - comp*2.0/3.0*etaN*one_dx_dz;
    }

    // dsyy/dz
    if ( mesh->BCv.type[c3-nxvz] != 30 ) {
        vS  =  2.0*one_dz_dz * etaS   - comp*2.0/3.0*etaS*one_dz_dz;
        vC  = -2.0*one_dz_dz * (etaS) + comp*2.0/3.0*etaS*one_dz_dz;
    }

    // d/dx eta*dvzdx
    if ( mesh->BCv.type[c3-1] != 30 && mesh->BCv.type[c3+1] != 30 ) {
        if ( mesh->BCv.type[c3-1] != 13 ) {vW  =  one_dx_dx * etaW; vC += -one_dx_dx * etaW;}
        if ( mesh->BCv.type[c3+1] != 13 ) {vE  =  one_dx_dx * etaE; vC += -one_dx_dx * etaE;}
    }
    // d/dx eta*dvzdx
    if ( mesh->BCv.type[c3-1] == 30 && mesh->BCv.type[c3+1] != 30 ) {
        if ( mesh->BCv.type[c3+1] != 13 ) vE  =  one_dx_dx * etaE;
        if ( mesh->BCv.type[c3+1] != 13 ) vC += -one_dx_dx * etaE;
    }
    // d/dx eta*dvzdx
    if ( mesh->BCv.type[c3-1] != 30 && mesh->BCv.type[c3+1] == 30 ) {
        if ( mesh->BCv.type[c3-1] != 13 ) vW  =  one_dx_dx * etaW;
        if ( mesh->BCv.type[c3-1] != 13 ) vC += -one_dx_dx * etaW;
    }

    if ( mesh->BCv.type[c3-1] == 11 ) vC  +=  -one_dx_dx * etaW;
    if ( mesh->BCv.type[c3+1] == 11 ) vC  +=  -one_dx_dx * etaE;

    // Stabilisation with density gradients
    if (stab==1) {
        double drhodz = (mesh->rho_n[c2+ncx] - mesh->rho_n[c2])*one_dz;
        //        double drhodx = (mesh->rho_s[c1]   - mesh->rho_s[c1-1])*one_dx;
        vC  += - 1.00 * om * model.dt * model.gz * drhodz;
        //        // Non-symmetric contibution to the system of equation
        //        uNW += - 0.25 * om * model.dt * model.gz * drhodx;
        //        uNE += - 0.25 * om * model.dt * model.gz * drhodx;
        //        uSW += - 0.25 * om * model.dt * model.gz * drhodx;
        //        uSE += - 0.25 * om * model.dt * model.gz * drhodx;
    }

    // Pressure gradient
    if (  mesh->BCp.type[c2] != 30 ) {
        pS  =   one_dz;
    }

    uSW=-uSW, uSE=-uSE, uNW=-uNW, uNE=-uNE, vS=-vS, vW=-vW, vC=-vC, vE=-vE, vN=-vN, pN=-pN, pS=-pS;

    if ( Assemble == 1 ) {
        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------
        if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1-1],     &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[c1-1],    mesh->BCu.val[c1-1],    StokesA->bbc );
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1],       &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[c1],      mesh->BCu.val[c1],      StokesA->bbc );
        }
        if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx-1],  &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[c1+nx-1], mesh->BCu.val[c1+nx-1], StokesA->bbc );
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[c1+nx],    &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[c1+nx],   mesh->BCu.val[c1+nx],   StokesA->bbc );
        }
        //--------------------
        if ( mesh->BCv.type[c3-nxvz] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-nxvz],  &(nnzc2A[ith]), vS*celvol, mesh->BCv.type[c3-nxvz], mesh->BCv.val[c3-nxvz], StokesA->bbc );
        if ( mesh->BCv.type[c3-1] != 30 ) {
            if( mesh->BCv.type[c3-1] != 11 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-1],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3-1],    mesh->BCv.val[c3-1],    StokesA->bbc );
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3-1],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[c3-1],    2.0*mesh->BCv.val[c3-1],    StokesA->bbc );
        }
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                     &(nnzc2A[ith]), vC*celvol, mesh->BCv.type[c3],      mesh->BCv.val[c3],      StokesA->bbc );
        if ( mesh->BCv.type[c3+1] != 30 ) {
            if ( mesh->BCv.type[c3+1] != 11 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3+1],    mesh->BCv.val[c3+1],    StokesA->bbc );
            else AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[c3+1],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[c3+1],    2.0*mesh->BCv.val[c3+1],    StokesA->bbc );
        }
        //--------------------
        if (  mesh->BCp.type[c2] != 30) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[c2] - Stokes->neq_mom,      &(nnzc2B[ith]), pS*celvol, mesh->BCp.type[c2],     mesh->BCp.val[c2],     StokesB->bbc );
        }
    }
    else {
        // RHS
        StokesA->F[eqn] = -(StokesA->b[eqn] + StokesA->bbc[eqn]/celvol);
        StokesA->F[eqn] += vC*v[c3];
        if (mesh->BCv.type[c3-1] != 30) {
            if (mesh->BCv.type[c3-1] != 11) StokesA->F[eqn] += vW*v[c3-1];
        }
        if (mesh->BCv.type[c3-nxvz]  != 30) StokesA->F[eqn] += vS*v[c3-nxvz];
        if (mesh->BCv.type[c3+1] != 30) {
            if (mesh->BCv.type[c3+1] != 11) StokesA->F[eqn] += vE*v[c3+1];
        }
        //--------------------
        if ( mesh->BCu.type[c1-1] != 30 && mesh->BCu.type[c1] != 30 ) {
            if (mesh->BCu.type[c1-1] != 11 && mesh->BCu.type[c1-1] != 0) StokesA->F[eqn] += uSW*u[c1-1];
            if (mesh->BCu.type[c1  ] != 11 && mesh->BCu.type[c1  ] != 0) StokesA->F[eqn] += uSE*u[c1  ];
        }
        if ( mesh->BCu.type[c1+nx-1] != 30 && mesh->BCu.type[c1+nx] != 30 ) {
            if (mesh->BCu.type[c1+nx-1] != 11 && mesh->BCu.type[c1+nx-1] != 0) StokesA->F[eqn] += uNW*u[c1+nx-1];
            if (mesh->BCu.type[c1+nx  ] != 11 && mesh->BCu.type[c1+nx  ] != 0) StokesA->F[eqn] += uNE*u[c1+nx];
        }
        //--------------------
        if ( mesh->BCp.type[c2-ncx] != 30  ) StokesA->F[eqn]  +=  pS*p[c2-ncx];
        StokesA->F[eqn] *= celvol;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void MergeParallelMatrixDecoupled( SparseMat *Stokes, double **Atemp, int **Jtemp, int **Itemp, grid* mesh, int* estart, int* eend, int *nnzc, int *nnzc2, int *last_eqn, int n_th, char* BC_type, int *eqn_number ) {

    int eqn, ith, begin[n_th], end[n_th], c, last_eqn_serial=0, k, l;

    for (ith=0; ith<n_th; ith++) {

        // Get last equation number
        if (last_eqn[ith]>last_eqn_serial) {
            last_eqn_serial = last_eqn[ith];
        }

        if (ith==0) {
            begin[ith] = *(nnzc);
            end[ith]   = *(nnzc) + nnzc2[ith]-1;
        }
        else {
            begin[ith] = end[ith-1]+1;
            end[ith]   = end[ith-1]+1+nnzc2[ith]-1;
        }
    }

    // Recombine A and J
#pragma omp parallel private(ith, eqn, c, k, l ) shared( Stokes, Atemp, Jtemp, Itemp, begin, BC_type, eqn_number )
    {
        ith = omp_get_thread_num();

        for( c=estart[ith]; c<eend[ith]+1; c++) {

            // Get equation numbering
            //            if ( BC_type[c] != 0 && BC_type[c] != 30 && BC_type[c] != 31 && BC_type[c] != 13 && BC_type[c] != 11 ) {
            if ( BC_type[c] != 0 && BC_type[c] != 30 && BC_type[c] != 31 && BC_type[c] != 13 && BC_type[c] != 11 && BC_type[c] != -12  ) {
                eqn = eqn_number[c] -  Stokes->neq_mom;
                Stokes->Ic[eqn] = Itemp[ith][eqn] + begin[ith];
            }
        }

        for (k=0; k<nnzc2[ith]; k++) {
            l = begin[ith] + k;
            Stokes->A[l] = Atemp[ith][k];
            Stokes->J[l] = Jtemp[ith][k];
        }
    }

    // Update total non-zero number
    for (ith=0; ith<n_th; ith++) {
        *(nnzc) += nnzc2[ith];
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void FreeDecompositionDecoupled( int *estart, int *eend, int *DD, int *last_eqn  ) {
    DoodzFree( DD );
    DoodzFree( estart );
    DoodzFree( eend );
    DoodzFree( last_eqn );
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void AllocateDecompositionDecoupled( int n_th, int **estart, int **eend, int **DD, int **last_eqn ) {

    /* Array containing the size of the block for each thread */
    *DD       = DoodzMalloc( sizeof(int) * n_th );
    *estart   = DoodzMalloc( sizeof(int) * n_th );
    *eend     = DoodzMalloc( sizeof(int) * n_th );
    *last_eqn = DoodzCalloc( n_th, sizeof(int) );
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void DomainDecompositionDecoupled( int N, int n_th, int *estart, int *eend, int *DD ) {

    int ith, eps, block_size, x0 = 0, x1 = 0;
    eps = N%n_th;
    block_size  = (N - eps) / n_th;

    // Divide domain into blocks
    for (ith=0; ith<n_th; ith++) {
        if (ith<n_th-1)
            DD[ith] = block_size;
        else {
            DD[ith] = block_size + eps;
        }

        x1 = x0 + DD[ith];
        estart[ith] = x0;
        eend[ith]   = x1-1;
        x0          = x1;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void AllocateTempMatArraysDecoupled(double ***Atemp, int ***Itemp, int ***Jtemp, int n_th, int nnz, int neq, int* DD, int **nnzc2 ) {

    int ith;

    // Temporary parallel Matrix arrays
    *Atemp = DoodzMalloc( sizeof(double*) * n_th );
    *Itemp = DoodzMalloc( sizeof(int*) * n_th );
    *Jtemp = DoodzMalloc( sizeof(int*) * n_th );
    *nnzc2 = DoodzCalloc( n_th, sizeof(int) );

    for (ith=0; ith<n_th; ith++) {
        (*Atemp)[ith] = DoodzMalloc( sizeof(double) * (int)(nnz/n_th) );
        (*Itemp)[ith] = DoodzCalloc( (neq+1), sizeof(int) );
        (*Jtemp)[ith] = DoodzMalloc( sizeof(int) * (int)(nnz/n_th) );
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void FreeTempMatArraysDecoupled(double **Atemp, int **Itemp, int **Jtemp, int n_th, int *nnzc2 ) {

    int ith;

    // Temporary parallel Matrix arrays
    for (ith=0; ith<n_th; ith++) {
        DoodzFree( Atemp[ith] );
        DoodzFree( Itemp[ith] );
        DoodzFree( Jtemp[ith] );
    }
    DoodzFree( Atemp );
    DoodzFree( Itemp );
    DoodzFree( Jtemp );
    DoodzFree( nnzc2 );
}


/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void BuildStokesOperatorDecoupled( grid *mesh, params model, int lev, double *p_corr, double *p, double *u, double *v, SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, SparseMat *StokesC, SparseMat *StokesD, int Assemble ) {

    // FLAG
    // Assemble=1: Build the linear system of discrete equations
    // Assemble=0: Evaluate Stokes residuals

    int    cc, k, l, c1, c2, c3, nx=mesh->Nx, nz=mesh->Nz, nxvz=nx+1, nzvx=nz+1, ncx=nx-1, ncz=nz-1;

    // Pre-calculate FD coefs
    double celvol    = mesh->dx*mesh->dz;
    double one_dx    = 1.0/mesh->dx;
    double one_dz    = 1.0/mesh->dz;
    double one_dx_dx = 1.0/mesh->dx/mesh->dx;
    double one_dz_dz = 1.0/mesh->dz/mesh->dz;
    double one_dx_dz = 1.0/mesh->dx/mesh->dz;

    // Switches
    int eqn,  sign = 1, comp = 0, stab = 0;
    if (model.free_surf_stab>0) stab = 1;
    if (model.compressible==1  ) comp = 1;
    double theta = model.free_surf_stab;

    //    if ( stab==1 ) printf("It tastes like Bo'\n");

    // Decompose domain
    int n_th, N, ith;
    int *DD, *estart, *eend, *last_eqn;

    // Temporary parallel matrix
    double **AtempA;
    int **JtempA, **ItempA;
    double **AtempB;
    int **JtempB, **ItempB;
    double **AtempC;
    int **JtempC, **ItempC;
    double **AtempD;
    int **JtempD, **ItempD;

    int nnzA, nnzB, nnzC, nnzD, nnzcA=0, nnzcB=0, nnzcC=0, nnzcD=0;
    int *nnzc2A, *nnzc2B, *nnzc2C, *nnzc2D;

#pragma omp parallel shared(n_th)
    {
        n_th = omp_get_num_threads();
    }
#pragma omp barrier

    // Matrix initialisation
    if ( Assemble == 1 ) {

        nnzA  = 9*((mesh->Nx-1) * mesh->Nz + (mesh->Nz-1) * mesh->Nx);
        nnzB  = 5*((mesh->Nx-1) * (mesh->Nz-1));
        nnzC  = nnzB;
        nnzD  = 1*((mesh->Nx-1) * (mesh->Nz-1));
        StokesA->neq = Stokes->neq_mom;
        StokesB->neq = Stokes->neq_mom;
        StokesC->neq = Stokes->neq_cont; StokesC->neq_mom = Stokes->neq_mom;
        StokesD->neq = Stokes->neq_cont; StokesD->neq_mom = Stokes->neq_mom;
        //printf("Assembling  decoupled Stokes matrix...\n");
        AllocMat( StokesA, nnzA );
        AllocMat( StokesB, nnzB );
        AllocMat( StokesC, nnzC );
        AllocMat( StokesD, nnzD );

    }

    // Build velocity block RHS
    int inc=0;
    for( l=0; l<nzvx; l++) {
        for( k=0; k<nx; k++) {
            cc = k + l*nx;
            if ( mesh->BCu.type[cc] != 0 && mesh->BCu.type[cc] != 30 && mesh->BCu.type[cc] != 11 && mesh->BCu.type[cc] != 13 && mesh->BCu.type[cc] != -12 ) {
                StokesA->b[inc] = mesh->roger_x[cc];
                inc++;
            }
        }
    }
    for( l=0; l<nz; l++) {
        for( k=0; k<nxvz; k++) {
            cc = k + l*nxvz;
            if ( mesh->BCv.type[cc] != 0 && mesh->BCv.type[cc] != 30 && mesh->BCv.type[cc] != 11 && mesh->BCv.type[cc] != 13  && mesh->BCv.type[cc] != -12  ) {
                StokesA->b[inc] = mesh->roger_z[cc];
                inc++;
            }
        }
    }

    // Build pressure block RHS
    inc=0;
    for( l=0; l<ncz; l++) {
        for( k=0; k<ncx; k++) {
            cc = k + l*ncx;
            if ( mesh->BCp.type[cc] != 31 && mesh->BCp.type[cc] != 0 && mesh->BCp.type[cc] != 30 ) {
                StokesC->b[inc] = mesh->rhs_p[cc];
                inc++;
            }
        }
    }

    //------------------------------------------------------------------//
    //------------------------- U-momentum -----------------------------//
    //------------------------------------------------------------------//

    // Parallel decomposition
    N = nx*nzvx;
    AllocateDecompositionDecoupled(  n_th, &estart, &eend, &DD, &last_eqn );
    DomainDecompositionDecoupled( N, n_th, estart, eend, DD );

    // Allocate parallel matrix
    if ( Assemble == 1 ) {
        AllocateTempMatArraysDecoupled( &AtempA, &ItempA, &JtempA, n_th, nnzA, Stokes->neq_mom, DD, &nnzc2A  );
        AllocateTempMatArraysDecoupled( &AtempB, &ItempB, &JtempB, n_th, nnzB, Stokes->neq_mom, DD, &nnzc2B  );
    }

#pragma omp parallel shared( eend, estart, mesh, Stokes, StokesA, StokesB, u, v, p_corr, nx, ncx, nzvx, nnzc2A, AtempA, JtempA, ItempA, nnzc2B, AtempB, JtempB, ItempB, last_eqn )  private( ith, l, k, c1, c2, c3, eqn ) firstprivate( model, Assemble, lev, one_dx_dx, one_dz_dz, one_dx_dz, one_dx, one_dz, sign, theta, stab, celvol, comp )
    {
        ith = omp_get_thread_num();

        for( c1=estart[ith]; c1<eend[ith]+1; c1++) {

            k      = mesh->kvx[c1];
            l      = mesh->lvx[c1];
            c2     = k-1 + (l-1)*ncx;
            c3     = k   + l*nxvz;

            // Get equation numbering
            if ( mesh->BCu.type[c1]==-1 || mesh->BCu.type[c1]==2 || mesh->BCu.type[c1]==-2 ) {
                eqn = Stokes->eqn_u[c1];
                last_eqn[ith]   = eqn;

                if ( Assemble == 1 ) {
                    ItempA[ith][eqn] = nnzc2A[ith];
                    ItempB[ith][eqn] = nnzc2B[ith];
                }

                //--------------------- INNER NODES ---------------------//
                if ( l>0 && l<nzvx-1 && k>0 && k<nx-1 ) {
                    Xmomentum_InnerNodesDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }

                // Periodic W
                if ( k==0    && mesh->BCu.type[c1]==-2 ) {
                    Xmomentum_InnerNodesDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }

                // Neumann W
                if ( k==0    && mesh->BCu.type[c1]==2 ) {
                    Xmomentum_WestNeumannDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }

                // Neumann E
                if ( k==nx-1 && mesh->BCu.type[c1]==2 ) {
                    Xmomentum_EastNeumannDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }
            }
        }
    }

    //-----------------------------------------------------------------//

    if ( Assemble == 1 ) {
        MergeParallelMatrix( StokesA, AtempA, JtempA, ItempA, mesh, estart, eend, &nnzcA, nnzc2A, last_eqn, n_th, mesh->BCu.type, Stokes->eqn_u );
        MergeParallelMatrix( StokesB, AtempB, JtempB, ItempB, mesh, estart, eend, &nnzcB, nnzc2B, last_eqn, n_th, mesh->BCu.type, Stokes->eqn_u );
    }

    if ( Assemble == 1 ) {
        FreeTempMatArraysDecoupled( AtempA, ItempA, JtempA, n_th, nnzc2A );
        FreeTempMatArraysDecoupled( AtempB, ItempB, JtempB, n_th, nnzc2B );
    }
    FreeDecompositionDecoupled( estart, eend, DD, last_eqn );

    //------------------------------------------------------------------//
    //------------------------- V-momentum -----------------------------//
    //------------------------------------------------------------------//

    // Parallel decomposition
    N = nxvz*nz;
    AllocateDecompositionDecoupled(  n_th, &estart, &eend, &DD, &last_eqn );
    DomainDecompositionDecoupled( N, n_th, estart, eend, DD );

    // Allocate parallel matrix
    if ( Assemble == 1 ) {
        AllocateTempMatArraysDecoupled( &AtempA, &ItempA, &JtempA, n_th, nnzA, Stokes->neq_mom, DD, &nnzc2A  );
        AllocateTempMatArraysDecoupled( &AtempB, &ItempB, &JtempB, n_th, nnzB, Stokes->neq_mom, DD, &nnzc2B  );
    }

#pragma omp parallel shared( eend, estart, mesh, Stokes, StokesA, StokesB, u, v, p_corr, nx, ncx, nzvx, nnzc2A, AtempA, JtempA, ItempA, nnzc2B, AtempB, JtempB, ItempB, last_eqn )  private( ith, l, k, c1, c2, c3, eqn ) firstprivate( model, Assemble, lev, one_dx_dx, one_dz_dz, one_dx_dz, one_dx, one_dz, sign, theta, stab, comp, celvol )
    {
        ith = omp_get_thread_num();

        for( c3=estart[ith]; c3<eend[ith]+1; c3++) {

            k  = mesh->kvz[c3];
            l  = mesh->lvz[c3];
            c1 = k   + l*nx;
            c2 = k-1 + (l-1)*ncx;

            // Get equation numbering
            if ( mesh->BCv.type[c3] == -1 || mesh->BCv.type[c3]==2 ) {
                eqn = Stokes->eqn_v[c3];
                last_eqn[ith]   = eqn;

                if ( Assemble == 1 ) {
                    ItempA[ith][eqn] = nnzc2A[ith];
                    ItempB[ith][eqn] = nnzc2B[ith];
                }

                //--------------------- INNER NODES ---------------------//
                if ( k>0 && k<nxvz-1 && l>0 && l<nz-1 ) {

                    Zmomentum_InnerNodesDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }

                //--------------------- INNER NODES ---------------------//

                if ( l==0 && mesh->BCv.type[c3] == 2 ) {
                    Zmomentum_SouthNeumannDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }

                //--------------------- INNER NODES ---------------------//

                if ( l==nz-1 && mesh->BCv.type[c3] == 2 ) {
                    Zmomentum_NorthNeumannDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }

            }
        }
    }

    //-----------------------------------------------------------------//

    if ( Assemble == 1 ) {
        MergeParallelMatrix( StokesA, AtempA, JtempA, ItempA, mesh, estart, eend, &nnzcA, nnzc2A, last_eqn, n_th, mesh->BCv.type, Stokes->eqn_v );
        MergeParallelMatrix( StokesB, AtempB, JtempB, ItempB, mesh, estart, eend, &nnzcB, nnzc2B, last_eqn, n_th, mesh->BCv.type, Stokes->eqn_v );
    }

    if ( Assemble == 1 ) {
        FreeTempMatArraysDecoupled( AtempA, ItempA, JtempA, n_th, nnzc2A );
        FreeTempMatArraysDecoupled( AtempB, ItempB, JtempB, n_th, nnzc2B );
    }
    FreeDecompositionDecoupled( estart, eend, DD, last_eqn );

    //------------------------------------------------------------------//
    //------------------------- Continuity -----------------------------//
    //------------------------------------------------------------------//

    // Parallel decomposition
    N = ncx*ncz;
    AllocateDecompositionDecoupled(  n_th, &estart, &eend, &DD, &last_eqn );
    DomainDecompositionDecoupled( N, n_th, estart, eend, DD );

    // Allocate parallel matrix
    if ( Assemble == 1 ) {
        AllocateTempMatArraysDecoupled( &AtempC, &ItempC, &JtempC, n_th, nnzC, Stokes->neq_cont, DD, &nnzc2C  );
        AllocateTempMatArraysDecoupled( &AtempD, &ItempD, &JtempD, n_th, nnzD, Stokes->neq_cont, DD, &nnzc2D  );
    }

    //    printf("NC = %d %d %d %d %d\n", Stokes->neq_cont, estart[0], eend[0], DD[0], last_eqn[0]);

    //#pragma omp parallel shared( eend, estart, mesh, Stokes, u, v, p, nx, ncx, nzvx, nnzc2C, AtempC, JtempC, ItempC, nnzc2D, AtempD, JtempD, ItempD, last_eqn )  private( ith, l, k, c1, c2, c3, eqn ) firstprivate( model, Assemble, lev, one_dx_dx, one_dz_dz, one_dx_dz, one_dx, one_dz, sign, theta, stab, comp, celvol )
    //    {
    //
    //        ith = omp_get_thread_num();
    //
    //        for( c2=estart[ith]; c2<eend[ith]+1; c2++) {
    //
    //            k   = mesh->kp[c2];
    //            l   = mesh->lp[c2];
    //            c1  = k   + (l+1)*nx;
    //            c3  = k   + l*nxvz + 1;
    //
    //            //--------------------- INNER NODES ---------------------//
    //            if ( mesh->BCp.type[c2] == -1) {
    //
    //
    //                eqn = Stokes->eqn_p[c2]  - Stokes->neq_mom;
    //                last_eqn[ith]   = eqn ;
    //
    //                if ( Assemble == 1 ) {
    //                    ItempC[ith][eqn] = nnzc2C[ith];
    //                    ItempD[ith][eqn] = nnzc2D[ith]; //printf("%d ",  nnzc2D[ith]);
    //                }
    //                //
    //                Continuity_InnerNodesDecoupled( Stokes, StokesC, StokesD, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p, JtempC, AtempC, nnzc2C, JtempD, AtempD, nnzc2D, k, l );
    //            }
    //        }
    //    }

#pragma omp parallel shared( eend, estart, mesh, Stokes, StokesC, StokesD, u, v, p, nnzc2C, AtempC, JtempC, ItempC, nnzc2D, AtempD, JtempD, ItempD, last_eqn )  private( ith, l, k, c1, c2, c3, eqn, comp ) firstprivate( model, Assemble, lev, one_dx_dx, one_dz_dz, one_dx_dz, one_dx, one_dz, sign, theta, stab, celvol, nx, ncx, nxvz, nzvx )
    {

        ith = omp_get_thread_num();

        for( c2=estart[ith]; c2<eend[ith]+1; c2++) {

            k   = mesh->kp[c2];
            l   = mesh->lp[c2];
            c1  = k   + (l+1)*nx;
            c3  = k   + l*nxvz + 1;

            //--------------------- INNER NODES ---------------------//
            if ( mesh->BCp.type[c2] == -1) {

                comp = mesh->comp_cells[c2];

                eqn = Stokes->eqn_p[c2]  - Stokes->neq_mom;
                last_eqn[ith]   = eqn ;

                if ( Assemble == 1 ) {
                    ItempC[ith][eqn] = nnzc2C[ith];
                    ItempD[ith][eqn] = nnzc2D[ith]; //printf("%d ",  nnzc2D[ith]);
                }
                //
                Continuity_InnerNodesDecoupled( Stokes, StokesC, StokesD, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p, JtempC, AtempC, nnzc2C, JtempD, AtempD, nnzc2D, k, l );
            }
        }
    }

    //-----------------------------------------------------------------//

    if ( Assemble == 1 ) {
        MergeParallelMatrixDecoupled( StokesC, AtempC, JtempC, ItempC, mesh, estart, eend, &nnzcC, nnzc2C, last_eqn, n_th, mesh->BCp.type, Stokes->eqn_p );
        MergeParallelMatrixDecoupled( StokesD, AtempD, JtempD, ItempD, mesh, estart, eend, &nnzcD, nnzc2D, last_eqn, n_th, mesh->BCp.type, Stokes->eqn_p );
    }
    if ( Assemble == 1 ) {
        FreeTempMatArraysDecoupled( AtempC, ItempC, JtempC, n_th, nnzc2C );
        FreeTempMatArraysDecoupled( AtempD, ItempD, JtempD, n_th, nnzc2D );
    }
    FreeDecompositionDecoupled( estart, eend, DD, last_eqn );

    //------------------------------------------------------------------//
    //----------------------------- End --------------------------------//
    //------------------------------------------------------------------//



    if ( Assemble == 1 ) {

        // Add contribution from the BC's
        for (k=0; k<StokesA->neq; k++) {
            StokesA->b[k] += StokesA->bbc[k];
            StokesB->b[k] = StokesA->b[k];
        }

        //            MinMaxArray(StokesA->b, 1, StokesA->neq, "rhs_mom_here" );
        //            MinMaxArray(StokesC->b, 1, StokesC->neq, "rhs_cont_here" );


        // Add contribution from the BC's
        for (k=0; k<StokesC->neq; k++) {
            StokesC->b[k] += StokesC->bbc[k];
            StokesD->b[k] = StokesC->b[k];
        }



        // Final index
        StokesA->Ic[StokesA->neq] = nnzcA;
        StokesB->Ic[StokesB->neq] = nnzcB;
        StokesC->Ic[StokesC->neq] = nnzcC;
        StokesD->Ic[StokesD->neq] = nnzcD;

        //        for (ieq=0; ieq<Stokes->neq_cont+1; ieq++) printf( "%d ", StokesC->I[ieq] );

        StokesA->nnz = nnzcA;
        StokesB->nnz = nnzcB;
        StokesC->nnz = nnzcC;
        StokesD->nnz = nnzcD;

        // Resize arrays to proper number of non-zeros
        double *bufd;
        int *bufi;
        bufi      = DoodzRealloc(StokesA->J, nnzcA*sizeof(int));
        bufd      = DoodzRealloc(StokesA->A, nnzcA*sizeof(double));
        StokesA->J = bufi;
        StokesA->A = bufd;

        bufi      = DoodzRealloc(StokesB->J, nnzcB*sizeof(int));
        bufd      = DoodzRealloc(StokesB->A, nnzcB*sizeof(double));
        StokesB->J = bufi;
        StokesB->A = bufd;

        bufi      = DoodzRealloc(StokesC->J, nnzcC*sizeof(int));
        bufd      = DoodzRealloc(StokesC->A, nnzcC*sizeof(double));
        StokesC->J = bufi;
        StokesC->A = bufd;

        bufi      = DoodzRealloc(StokesD->J, nnzcD*sizeof(int));
        bufd      = DoodzRealloc(StokesD->A, nnzcD*sizeof(double));
        StokesD->J = bufi;
        StokesD->A = bufd;


//        printf("System size: ndof = %d, nzA = %d nzB = %d nzC = %d nzD = %d\n", Stokes->neq, nnzcA, nnzcB, nnzcC, nnzcD);

        printf("Number of momentum equations: %d\n", StokesA->neq);

//        // Extract Diagonal of A - Viscous block
//        int i, j, locNNZ;
//        int I1, J1;
//
//        if (model.diag_scaling == 1) {
//
//#pragma omp parallel for shared(StokesA) private(I1,J1, i, j, locNNZ )
//        for (i=0;i<StokesA->neq; i++) {
//            I1     = StokesA->Ic[i];
//            locNNZ = StokesA->Ic[i+1] - StokesA->Ic[i];
//            for (J1=0;J1<locNNZ; J1++) {
//                 j = StokesA->J[I1 + J1];
//                if (i==j) StokesA->d[i] = StokesA->A[I1 + J1];
//             }
//        }
//        // Extract Diagonal of D - Pressure block
//#pragma omp parallel for shared(StokesD) private(i)
//        for (i=0;i<StokesD->neq; i++) {
//            StokesC->d[i] = 1.0;
//        }
//
//        // Scale A
//#pragma omp parallel for shared(StokesA) private(I1,J1, i, j, locNNZ )
//        for (i=0;i<StokesA->neq; i++) {
//
//            StokesA->b[i] *= StokesA->d[i];  // scale RHS
//
//            I1     = StokesA->Ic[i];
//            locNNZ = StokesA->Ic[i+1] - StokesA->Ic[i];
//            for (J1=0;J1<locNNZ; J1++) {
//                j = StokesA->J[I1 + J1];
//                StokesA->A[I1 + J1] *= StokesA->d[i]*StokesA->d[j];
//            }
//        }
//
//        // Scale B
//#pragma omp parallel for shared(StokesB,StokesA) private(I1,J1, i, j, locNNZ )
//        for (i=0;i<StokesB->neq; i++) {
//            I1     = StokesB->Ic[i];
//            locNNZ = StokesB->Ic[i+1] - StokesB->Ic[i];
//            for (J1=0;J1<locNNZ; J1++) {
//                j = StokesB->J[I1 + J1];
//                StokesB->A[I1 + J1] *= StokesA->d[i]*StokesC->d[j];
//            }
//        }
//
//        // Scale C
//#pragma omp parallel for shared(StokesC,StokesA) private(I1,J1, i, j, locNNZ )
//        for (i=0;i<StokesC->neq; i++) {
//
//            StokesC->b[i] *= StokesC->d[i]; // scale RHS
//
//            I1     = StokesC->Ic[i];
//            locNNZ = StokesC->Ic[i+1] - StokesC->Ic[i];
//            for (J1=0;J1<locNNZ; J1++) {
//                j = StokesC->J[I1 + J1];
//                StokesC->A[I1 + J1] *= StokesC->d[i]*StokesA->d[j];
//            }
//        }
//
//    }
//
//        MinMaxArray(StokesA->d, 1, StokesA->neq, "diag. A" );

//        printf("\n");
//         MinMaxArray(StokesA->A, 1, StokesA->nnz, "A" );
//        printf("\n");


        //--------------------------------------//

#ifndef _VG_
        if ( model.write_debug == 1 ) {

            char *filename;
            asprintf( &filename, "Stokes_%02dcpu_step%02d_iter%02d.gzip.h5", n_th, model.step, model.nit );
            printf("Writing Stokes matrix file: %s to disk...\n", filename);

            // Fill in DD data structure
            OutputSparseMatrix OutputDDA, OutputDDB, OutputDDC, OutputDDD;

            OutputDDA.V = StokesA->A;
            OutputDDA.Ic = StokesA->Ic;
            OutputDDA.J = StokesA->J;
            OutputDDA.b = StokesA->b;
            OutputDDA.F = StokesA->F;
            OutputDDB.V = StokesB->A;
            OutputDDB.Ic = StokesB->Ic;
            OutputDDB.J = StokesB->J;
            OutputDDB.b = StokesB->b;
            OutputDDC.V = StokesC->A;
            OutputDDC.Ic = StokesC->Ic;
            OutputDDC.J = StokesC->J;
            OutputDDC.b = StokesC->b;
            OutputDDC.F = StokesC->F;
            OutputDDD.V = StokesD->A;
            OutputDDD.Ic = StokesD->Ic;
            OutputDDD.J = StokesD->J;
            OutputDDD.b = StokesD->b;
            OutputDDA.eta_cell = mesh->eta_n;
            OutputDDA.params[0] = nx;
            OutputDDA.params[1] = nz;
            OutputDDA.params[2] = mesh->dx;
            OutputDDA.params[3] = mesh->dz;
            OutputDDA.eqn_u = DoodzMalloc(nx*nzvx*sizeof(int));
            OutputDDA.eqn_v = DoodzMalloc(nxvz*nz*sizeof(int));
            OutputDDA.eqn_p = DoodzMalloc(ncx*ncz*sizeof(int));

            for( l=0; l<nzvx; l++) {
                for( k=0; k<nx; k++) {
                    cc = k + l*nx;
                    c1 = k + l*nx;
                    //                OutputDD.eqn_u[cc]=c1;
                    OutputDDA.eqn_u[cc]=Stokes->eqn_u[cc];
                }
            }
            for( l=0; l<nz; l++) {
                for( k=0; k<nxvz; k++) {
                    cc = k + l*nxvz;
                    c3 = nx*nzvx + k + l*nxvz;
                    //                OutputDD.eqn_v[cc]=c3;
                    OutputDDA.eqn_v[cc]=Stokes->eqn_v[cc];
                }
            }
            for( l=0; l<ncz; l++) {
                for( k=0; k<ncx; k++) {
                    cc = k + l*ncx;
                    c2 = nx*nzvx + nxvz*nz + k + l*ncx;
                    //                OutputDD.eqn_p[cc]=c2;
                    OutputDDA.eqn_p[cc]=Stokes->eqn_p[cc];
                }
            }

            // Send data to file
            create_output_hdf5( filename );
            AddGroup_to_hdf5( filename, "model" );
            AddGroup_to_hdf5( filename, "matrix" );
            AddGroup_to_hdf5( filename, "numbering" );
            AddGroup_to_hdf5( filename, "fields" );
            AddFieldToGroup_generic( _TRUE_, filename, "model", "params" , 'd', 4, OutputDDA.params,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "numbering", "eqn_u" , 'i', nx*nzvx, OutputDDA.eqn_u,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "numbering", "eqn_v" , 'i', nxvz*nz, OutputDDA.eqn_v,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "numbering", "eqn_p" , 'i', ncx*ncz, OutputDDA.eqn_p,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "eta_n" , 'd', ncx*ncz, mesh->eta_n,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "eta_s" , 'd', nx*nz, mesh->eta_s,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "tag_s" , 'c', nx*nz, mesh->BCg.type,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "tag_n" , 'c', ncx*ncz, mesh->BCp.type,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "tag_u" , 'c', nx*nzvx, mesh->BCu.type,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "tag_v" , 'c', nxvz*nz, mesh->BCv.type,  1 );

            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "IA" , 'i', StokesA->neq+1, OutputDDA.Ic,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "JA" , 'i', nnzcA,  OutputDDA.J,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "VA" , 'd', nnzcA,  OutputDDA.V,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "IB" , 'i', StokesB->neq+1, OutputDDB.Ic,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "JB" , 'i', nnzcB,  OutputDDB.J,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "VB" , 'd', nnzcB,  OutputDDB.V,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "IC" , 'i', StokesC->neq+1, OutputDDC.Ic,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "JC" , 'i', nnzcC,  OutputDDC.J,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "VC" , 'd', nnzcC,  OutputDDC.V,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "ID" , 'i', StokesD->neq+1, OutputDDD.Ic,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "JD" , 'i', nnzcD,  OutputDDD.J,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "VD" , 'd', nnzcD,  OutputDDD.V,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "eta_cell", 'd', ncx*ncz, OutputDDA.eta_cell,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "rhs_mom" , 'd', StokesA->neq, OutputDDA.b,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "rhs_cont", 'd', StokesC->neq, OutputDDC.b,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "F_mom" , 'd', StokesA->neq, OutputDDA.F,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "F_cont", 'd', StokesC->neq, OutputDDC.F,  1 );
            //            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "rhs", 'd', Stokes->neq, OutputDD.b,  1 );
            DoodzFree(OutputDDA.eqn_u);
            DoodzFree(OutputDDA.eqn_v);
            DoodzFree(OutputDDA.eqn_p);
            free(filename);
        }
#endif
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/
# if 0 // before modifications
void Xjacobian_InnerNodesDecoupled3( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B, int k, int l  ) {


    int iVxC   = c1;
    int iVxW   = iVxC-1;
    int iVxE   = iVxC+1;
    int iVxS   = iVxC-nx;
    int iVxN   = iVxC+nx;
    int iVxSW  = iVxS-1;
    int iVxSE  = iVxS+1;
    int iVxNW  = iVxN-1;
    int iVxNE  = iVxN+1;
    int iVzSW  = c3-nxvz;
    int iVzSE  = c3-nxvz+1;
    int iVzNW  = c3;
    int iVzNE  = c3+1;
    int iVzSWW = iVzSW-1;
    int iVzSEE = iVzSE+1;
    int iVzNWW = iVzNW-1;
    int iVzNEE = iVzNE+1;
    int iVzNNW = iVzNW+nxvz, iVzSSW = iVzSW-nxvz;
    int iVzNNE = iVzNE+nxvz, iVzSSE = iVzSE-nxvz;

    int iPrW  = c2;
    int iPrE  = c2+1;
    int ixyN  = c1;
    int ixyS  = c1-nx;
    int ixySW = ixyS-1;
    int ixySE = ixyS+1;
    int ixyNW = ixyN-1;
    int ixyNE = ixyN+1;
    int iPrSW = iPrW - ncx;
    int iPrSE = iPrE - ncx;
    int iPrNW = iPrW + ncx;
    int iPrNE = iPrE + ncx;

    // Periodic ends of stencil for inner points
    if (mesh->BCv.type[iVzSWW] == -12)  iVzSWW = iVzSW + (nx-2);
    if (mesh->BCv.type[iVzNWW] == -12)  iVzNWW = iVzNW + (nx-2);
    if (mesh->BCv.type[iVzSEE] == -12)  iVzSEE = iVzSE - (nx-2);
    if (mesh->BCv.type[iVzNEE] == -12)  iVzNEE = iVzNE - (nx-2);

    if (mesh->BCu.type[iVxC] == -2) {
        iVxW   = c1+nx-2;      iPrW = c2+ncx; iVzSW = c3-2; iVzNW = c3+nxvz-2;
        iPrNW  = iPrW+ncx;    iPrSW = iPrW-ncx;
        iVxSW  = iVxW-nx;     iVxNW = iVxW+nx;
        iVzNNW = iVzNW+nxvz; iVzSSW = iVzSW-nxvz;
        iVzSWW = iVzSW-1;    iVzNWW = iVzNW-1;
    }



    int Newton = 1;

    double  uC=0.0;
    double  uS=0.0,  uN=0.0,  uW=0.0,  uE=0.0,  vSW=0.0,  vSE=0.0,  vNW=0.0,  vNE=0.0, pE=0.0, pW=0.0;
    double uSW=0.0, uSE=0.0, uNW=0.0, uNE=0.0, vSWW=0.0, vSEE=0.0, vNWW=0.0, vNEE=0.0, vSSW=0.0, vSSE = 0.0, vNNW=0.0, vNNE=0.0;
    double pSW=0.0, pSE=0.0, pNW=0.0, pNE=0.0;

    double D11E = mesh->D11_n[iPrE];
    double D12E = mesh->D12_n[iPrE];
    double D13E = mesh->D13_n[iPrE];
    double D14E = mesh->D14_n[iPrE];

    double D11W = mesh->D11_n[iPrW];
    double D12W = mesh->D12_n[iPrW];
    double D13W = mesh->D13_n[iPrW];
    double D14W = mesh->D14_n[iPrW];

    double D31N = mesh->D31_s[ixyN];
    double D32N = mesh->D32_s[ixyN];
    double D33N = mesh->D33_s[ixyN];
    double D34N = mesh->D34_s[ixyN];

    double D31S = mesh->D31_s[ixyS];
    double D32S = mesh->D32_s[ixyS];
    double D33S = mesh->D33_s[ixyS];
    double D34S = mesh->D34_s[ixyS];

    //    if ( mesh->BCu.type[iVxN] == 13 ) {D31N = 0.0; D32N = 0.0; D33N = 0.0; D34N = 0.0;}
    //    if ( mesh->BCu.type[iVxS] == 13 ) {D31S = 0.0; D32S = 0.0; D33S = 0.0; D34S = 0.0;}

    double inE=0.0, inW=0.0, inS=0.0, inN = 0.0;

    if (mesh->BCp.type[iPrW] == -1) inW = 1.0;
    if (mesh->BCp.type[iPrE] == -1) inE = 1.0;

    if (mesh->BCg.type[ixyS] != 30 && mesh->BCu.type[iVxS] != 13) inS = 1.0;
    if (mesh->BCg.type[ixyN] != 30 && mesh->BCu.type[iVxN] != 13) inN = 1.0;

    // X-tra
    double wE=0.0, wW=0.0, wS=0.0, wN = 0.0;
    double inSWc=0.0,inSEc=0.0,inNWc=0.0,inNEc=0.0;
    double inSWv=0.0,inSEv=0.0,inNWv=0.0,inNEv=0.0;
    int nzvx = model.Nz+1, nz = model.Nz;

    if (l>1 || (l==1 && mesh->BCu.type[iVxS] == 11 ) ) {
        if (mesh->BCp.type[iPrSW] == -1) inSWc = 1.0;
        if (mesh->BCp.type[iPrSE] == -1) inSEc = 1.0;
    }

    if (l<nzvx-2 || (l==nzvx-2 && mesh->BCu.type[iVxN] == 11 )) {
        if (mesh->BCp.type[iPrNW] == -1) inNWc = 1.0;
        if (mesh->BCp.type[iPrNE] == -1) inNEc = 1.0;
    }

    if ( (k>0)  || (k==0 && (mesh->BCv.type[iVzSW] == -1 || mesh->BCv.type[iVzSW] == 0)) ) {

        //        if (l ==1 &&k==0 && mesh->BCv.type[iVzSW] == 0) printf("DDDD\n");
        if (mesh->BCg.type[ixySW] != 30) inSWv = 1.0;   // modify for periodic
        if (mesh->BCg.type[ixyNW] != 30) inNWv = 1.0;  // modify for periodic
    }
    if ( (k<nx-1) || (k==nx-1 && (mesh->BCv.type[iVzSE] == -1 || mesh->BCv.type[iVzSE] == 0) ) ) {
        if (mesh->BCg.type[ixySE] != 30) inSEv = 1.0;   // modify for periodic
        if (mesh->BCg.type[ixyNE] != 30) inNEv = 1.0;   // modify for periodic
    }

    wE = inN + inS + inNEv + inSEv;
    wW = inN + inS + inNWv + inSWv;
    wS = inW + inE + inSWc + inSEc;
    wN = inW + inE + inNWc + inNEc;


    //    if(k==0) printf("%2.2e %2.2e %2.2e %2.2e type %d\n", wW, wE, wS, wN, mesh->BCv.type[iVzSW]);
    //    if (k==1)  printf("%d %d\n", mesh->BCv.type[iVzSWW],mesh->BCv.type[iVzNWW]);
    //    if ((k==0) &&  mesh->BCv.type[iVzNWW] == -1) printf("East in\n");
    //    if ((k==1) &&  mesh->BCv.type[iVzNWW] == -1) printf("East out\n");

    if (wW>1.0) wW = 1.0/wW;
    if (wE>1.0) wE = 1.0/wE;
    if (wS>1.0) wS = 1.0/wS;
    if (wN>1.0) wN = 1.0/wN;

    //    if (l==nzvx-2) ;

    //    if (l==nzvx-2) {
    //        printf("%f %f %f %f %d %f %f %f %f\n", wW,wE,wS,wN, k, inN, inS, inNWv, inSWv);
    //    }

    //    if (l==nzvx-2) {
    //        printf("N %f %f \n", wW,wE);
    //
    //    }
    //
    //    if (l==1) {
    //        printf("S %f %f \n", wW,wE);
    //
    //    }
    //
    //    if (k==0) {
    //        printf("W %f %f \n", wW,wE);
    //
    //    }
    //
    //    if (k==nx-2) {
    //        printf("E %f %f \n", wW,wE);
    //
    //    }

    double dx = mesh->dx;
    double dz = mesh->dz;

    uW = (1.0/3.0)*inW*(-3*D13W*dx*wW*(inNWv - inSWv) + dx*(-D31N*inN*wN*(comp - 3) + D31S*inS*wS*(comp - 3) - D32N*comp*inN*wN + D32S*comp*inS*wS) + dz*(D11W*(comp - 3) + D12W*comp))/(pow(dx, 2)*dz);
    uC = (1.0/3.0)*(dx*(3*dx*(D33N*inN + D33S*inS) + dz*(inE - inW)*(-D31N*inN*wN*(comp - 3) + D31S*inS*wS*(comp - 3) - D32N*comp*inN*wN + D32S*comp*inS*wS)) - dz*(3*dx*(inN - inS)*(-D13E*inE*wE + D13W*inW*wW) + dz*(D11E*inE*(comp - 3) + D11W*inW*(comp - 3) + D12E*comp*inE + D12W*comp*inW)))/(pow(dx, 2)*pow(dz, 2));
    uE = (1.0/3.0)*inE*(3*D13E*dx*wE*(inNEv - inSEv) + dx*(D31N*inN*wN*(comp - 3) - D31S*inS*wS*(comp - 3) + D32N*comp*inN*wN - D32S*comp*inS*wS) + dz*(D11E*(comp - 3) + D12E*comp))/(pow(dx, 2)*dz);
    uS = (1.0/3.0)*inS*(-3*D33S*dx + dz*wS*(inSEc - inSWc)*(D31S*(comp - 3) + D32S*comp) + 3*dz*(D13E*inE*wE - D13W*inW*wW))/(dx*pow(dz, 2));
    uN = (1.0/3.0)*inN*(-3*D33N*dx - dz*wN*(inNEc - inNWc)*(D31N*(comp - 3) + D32N*comp) + 3*dz*(-D13E*inE*wE + D13W*inW*wW))/(dx*pow(dz, 2));
    vSW = (1.0/3.0)*(-dx*(3*D33S*dz*inS + dx*(D31N*comp*inN*inW*wN + D31S*comp*inS*wS*(inSWc - inW) + D32N*inN*inW*wN*(comp - 3) + D32S*inS*wS*(comp - 3)*(inSWc - inW))) + dz*(dx*inW*(D11W*comp + D12W*(comp - 3)) + 3*dz*(D13E*inE*inS*wE - D13W*inW*wW*(inS - inSWv))))/(pow(dx, 2)*pow(dz, 2));
    vSE = (1.0/3.0)*(dx*(3*D33S*dz*inS + dx*(-D31N*comp*inE*inN*wN + D31S*comp*inS*wS*(inE - inSEc) - D32N*inE*inN*wN*(comp - 3) + D32S*inS*wS*(comp - 3)*(inE - inSEc))) - dz*(dx*inE*(D11E*comp + D12E*(comp - 3)) + 3*dz*(D13E*inE*wE*(inS - inSEv) - D13W*inS*inW*wW)))/(pow(dx, 2)*pow(dz, 2));
    vNW = (1.0/3.0)*(dx*(3*D33N*dz*inN - dx*(D31N*comp*inN*wN*(inNWc - inW) + D31S*comp*inS*inW*wS + D32N*inN*wN*(comp - 3)*(inNWc - inW) + D32S*inS*inW*wS*(comp - 3))) - dz*(dx*inW*(D11W*comp + D12W*(comp - 3)) + 3*dz*(-D13E*inE*inN*wE + D13W*inW*wW*(inN - inNWv))))/(pow(dx, 2)*pow(dz, 2));
    vNE = (1.0/3.0)*(-dx*(3*D33N*dz*inN + dx*(-D31N*comp*inN*wN*(inE - inNEc) + D31S*comp*inE*inS*wS - D32N*inN*wN*(comp - 3)*(inE - inNEc) + D32S*inE*inS*wS*(comp - 3))) + dz*(dx*inE*(D11E*comp + D12E*(comp - 3)) + 3*dz*(-D13E*inE*wE*(inN - inNEv) + D13W*inN*inW*wW)))/(pow(dx, 2)*pow(dz, 2));
    uSW = (1.0/3.0)*(-3*D13W*inSWv*inW*wW + D31S*inS*inSWc*wS*(comp - 3) + D32S*comp*inS*inSWc*wS)/(dx*dz);
    uSE = (1.0/3.0)*(3*D13E*inE*inSEv*wE - D31S*inS*inSEc*wS*(comp - 3) - D32S*comp*inS*inSEc*wS)/(dx*dz);
    uNW = (1.0/3.0)*(3*D13W*inNWv*inW*wW - D31N*inN*inNWc*wN*(comp - 3) - D32N*comp*inN*inNWc*wN)/(dx*dz);
    uNE = (1.0/3.0)*(-3*D13E*inE*inNEv*wE + D31N*inN*inNEc*wN*(comp - 3) + D32N*comp*inN*inNEc*wN)/(dx*dz);
    vSWW = -D13W*inSWv*inW*wW/pow(dx, 2);
    vSEE = -D13E*inE*inSEv*wE/pow(dx, 2);
    vNWW = -D13W*inNWv*inW*wW/pow(dx, 2);
    vNEE = -D13E*inE*inNEv*wE/pow(dx, 2);
    vSSW = (1.0/3.0)*inS*inSWc*wS*(D31S*comp + D32S*(comp - 3))/pow(dz, 2);
    vSSE = (1.0/3.0)*inS*inSEc*wS*(D31S*comp + D32S*(comp - 3))/pow(dz, 2);
    vNNW = (1.0/3.0)*inN*inNWc*wN*(D31N*comp + D32N*(comp - 3))/pow(dz, 2);
    vNNE = (1.0/3.0)*inN*inNEc*wN*(D31N*comp + D32N*(comp - 3))/pow(dz, 2);

    pW  =  -inW*one_dx + (inW*( D14W*dz + dx*(-D34N*inN*wN + D34S*inS*wS))/(dx*dz));
    pE  =   inE*one_dx + (inE*(-D14E*dz + dx*(-D34N*inN*wN + D34S*inS*wS))/(dx*dz));
    pSW = (D34S*inS*inSWc*wS/dz);
    pSE = ( D34S*inS*inSEc*wS/dz);
    pNW = (-D34N*inN*inNWc*wN/dz);
    pNE = (-D34N*inN*inNEc*wN/dz);

    // Stabilisation with density gradients
    if (stab==1) {
        double drhodx  = (mesh->rho_n[c2+1] - mesh->rho_n[c2])*one_dx;
        double uC_corr = 1.00 * om * model.dt * mesh->gx[c1] * drhodx;
        // Importante trique, voire meme gigantesque!
        if (uC+uC_corr>0.0) uC += uC_corr;
    }

    // Add contribution from non-conforming Dirichlets
    if ( mesh->BCu.type[iVxS]   == 11 ) uC  -=  uS ;
    if ( mesh->BCu.type[iVxN]   == 11 ) uC  -=  uN ;
    if ( mesh->BCu.type[iVxSW]  == 11 ) uW  -=  uSW;
    if ( mesh->BCu.type[iVxSE]  == 11 ) uE  -=  uSE;
    if ( mesh->BCu.type[iVxNW]  == 11 ) uW  -=  uNW;
    if ( mesh->BCu.type[iVxNE]  == 11 ) uE  -=  uNE;
    if ( mesh->BCv.type[iVzNWW] == 11 ) vNW -= vNWW;
    if ( mesh->BCv.type[iVzNEE] == 11 ) vNE -= vNEE;
    if ( mesh->BCv.type[iVzSWW] == 11 ) vSW -= vSWW;
    if ( mesh->BCv.type[iVzSEE] == 11 ) vSE -= vSEE;

    //    comp = 1.0;
    //
    //    // simpler
    //    uW = D11W*inW*((1.0/3.0)*comp/dx - 1.0/dx)/dx;
    //    uC = -(-D33N*inN/dz - D33S*inS/dz)/dz - (D11E*inE*((1.0/3.0)*comp/dx - 1/dx) - D11W*inW*(-1.0/3.0*comp/dx + 1.0/dx))/dx;
    //    uE = -D11E*inE*(-1.0/3.0*comp/dx + 1.0/dx)/dx;
    //    uS = -D33S*inS/pow(dz, 2.0);
    //    uN = -D33N*inN/pow(dz, 2.0);
    //    vSW = (1.0/3.0)*D11W*comp*inW/(dx*dz) - D33S*inS/(dx*dz);
    //    vSE = -1.0/3.0*D11E*comp*inE/(dx*dz) + D33S*inS/(dx*dz);
    //    vNW = -1.0/3.0*D11W*comp*inW/(dx*dz) + D33N*inN/(dx*dz);
    //    vNE = (1.0/3.0)*D11E*comp*inE/(dx*dz) - D33N*inN/(dx*dz);
    //
    //    // Add contribution from non-conforming Dirichlets
    //    if ( mesh->BCu.type[iVxS] == 11 ) uC  -=  uS;
    //    if ( mesh->BCu.type[iVxN] == 11 ) uC  -=  uN;
    //
    //    // Pressure gradient
    //    if ( mesh->BCp.type[iPrE] != 30 && mesh->BCp.type[iPrW] != 30 ) {
    //        pW  =  -one_dx;
    //        pE  =   one_dx;
    //    }

    //    if (k==0) printf("%d %d %d\n", mesh->BCu.type[iVxS], mesh->BCu.type[iVxSW], mesh->BCu.type[iVxSE]  );

    if ( Assemble == 1 ) {
        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------
        // dsxx/dx - normal stencil

        // uSW (Newton)
        if (mesh->BCu.type[iVxSW] != 30) {
            if (mesh->BCu.type[iVxSW] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSW], &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[iVxSW],     mesh->BCu.val[iVxSW], StokesA->bbc);
            else                             AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSW], &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[iVxSW], 2.0*mesh->BCu.val[iVxSW], StokesA->bbc);
        }

        // uS
        if (mesh->BCu.type[iVxS] != 30) {
            if (mesh->BCu.type[iVxS] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxS], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[iVxS],     mesh->BCu.val[iVxS], StokesA->bbc);
            else                            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxS], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[iVxS], 2.0*mesh->BCu.val[iVxS], StokesA->bbc);
        }

        // uSE (Newton)
        if (mesh->BCu.type[iVxSE] != 30) {
            if (mesh->BCu.type[iVxSE] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSE], &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[iVxSE],     mesh->BCu.val[iVxSE], StokesA->bbc);
            else                             AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSE], &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[iVxSE], 2.0*mesh->BCu.val[iVxSE], StokesA->bbc);
        }

        // uW
        if (mesh->BCu.type[iVxW]  != 30)     AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxW],  &(nnzc2A[ith]), uW*celvol, mesh->BCu.type[iVxW],  mesh->BCu.val[iVxW],  StokesA->bbc);

        // uC
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                  &(nnzc2A[ith]), uC*celvol, mesh->BCu.type[iVxC],    mesh->BCu.val[iVxC],    StokesA->bbc);

        // uE
        if (mesh->BCu.type[iVxE]  != 30)     AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxE],  &(nnzc2A[ith]), uE*celvol, mesh->BCu.type[iVxE],  mesh->BCu.val[iVxE],  StokesA->bbc);

        // uNW (Newton)
        if (mesh->BCu.type[iVxNW] != 30) {
            if (mesh->BCu.type[iVxNW] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNW], &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[iVxNW],     mesh->BCu.val[iVxNW], StokesA->bbc);
            else                             AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNW], &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[iVxNW], 2.0*mesh->BCu.val[iVxNW], StokesA->bbc);
        }

        // uN
        if (mesh->BCu.type[iVxN] != 30) {
            if (mesh->BCu.type[iVxN] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxN], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[iVxN],   mesh->BCu.val[iVxN], StokesA->bbc);
            else                            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxN], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[iVxN], 2*mesh->BCu.val[iVxN], StokesA->bbc);
        }

        // uNE (Newton)
        if (mesh->BCu.type[iVxNE] != 30) {
            if (mesh->BCu.type[iVxNE] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNE], &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[iVxNE],     mesh->BCu.val[iVxNE], StokesA->bbc);
            else                             AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNE], &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[iVxNE], 2.0*mesh->BCu.val[iVxNE], StokesA->bbc);
        }

        //--------------------

        // vSSW
        if ( l>1 ) if ( mesh->BCv.type[iVzSSW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSSW],      &(nnzc2A[ith]), vSSW*celvol, mesh->BCv.type[iVzSSW],      mesh->BCv.val[iVzSSW],      StokesA->bbc);

        // vSSE
        if ( l>1 ) if ( mesh->BCv.type[iVzSSE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSSE],      &(nnzc2A[ith]), vSSE*celvol, mesh->BCv.type[iVzSSE],      mesh->BCv.val[iVzSSE],      StokesA->bbc);

        //--------------------


        // vSWW (Newton)
        if ( (k > 1 ) ||  ( (k==0 || k==1) &&  mesh->BCv.type[iVzSWW] == -1) ) {
            if (mesh->BCv.type[iVzSWW] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSWW],   &(nnzc2A[ith]), vSWW*celvol, mesh->BCv.type[iVzSWW],   mesh->BCv.val[iVzSWW],   StokesA->bbc);
        }

        // vSW && vSE
        if ( mesh->BCv.type[iVzSW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSW], &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[iVzSW], mesh->BCv.val[iVzSW],   StokesA->bbc);
        if ( mesh->BCv.type[iVzSE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSE], &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[iVzSE], mesh->BCv.val[iVzSE], StokesA->bbc);

        // vSEE (Newton)
        if ( (k < nx-2 ) ||  ( (k==nx-2 || k==nx-1) &&  mesh->BCv.type[iVzSEE] == -1) ) {
            if ( mesh->BCv.type[iVzSEE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSEE], &(nnzc2A[ith]), vSEE*celvol, mesh->BCv.type[iVzSEE], mesh->BCv.val[iVzSEE], StokesA->bbc);
        }

        // vNWW (Newton)
        if ( (k > 1 ) ||  ( (k==0 || k==1) &&  mesh->BCv.type[iVzNWW] == -1) ) {
            if ( mesh->BCv.type[iVzNWW] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNWW],        &(nnzc2A[ith]), vNWW*celvol, mesh->BCv.type[iVzNWW],        mesh->BCv.val[iVzNWW],        StokesA->bbc);
        }

        // vNW && vNE
        if ( mesh->BCv.type[iVzNW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNW],      &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[iVzNW],      mesh->BCv.val[iVzNW],      StokesA->bbc);
        if ( mesh->BCv.type[iVzNE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNE],      &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[iVzNE],      mesh->BCv.val[iVzNE],      StokesA->bbc);

        // vNEE (Newton)
        if ( (k < nx-2 ) ||  ((k==nx-2 || k==nx-1) &&  mesh->BCv.type[iVzNEE] == -1) ) {
            if ( mesh->BCv.type[iVzNEE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNEE],      &(nnzc2A[ith]), vNEE*celvol, mesh->BCv.type[iVzNEE],      mesh->BCv.val[iVzNEE],      StokesA->bbc);
        }

        //--------------------

        // vNNW
        if ( l<nz-1 ) if ( mesh->BCv.type[iVzNNW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNNW],      &(nnzc2A[ith]), vNNW*celvol, mesh->BCv.type[iVzNNW],      mesh->BCv.val[iVzNNW],      StokesA->bbc);

        // vNNE
        if ( l<nz-1) if ( mesh->BCv.type[iVzNNE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNNE],      &(nnzc2A[ith]), vNNE*celvol, mesh->BCv.type[iVzNNE],      mesh->BCv.val[iVzNNE],      StokesA->bbc);

        //--------------------

        if ( Newton==1 && l>1) {
            // pSW
            if ( mesh->BCp.type[iPrSW] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrSW] - Stokes->neq_mom,   &(nnzc2B[ith]), pSW*celvol, mesh->BCp.type[iPrSW],   mesh->BCp.val[iPrSW],   StokesB->bbc);
            }

            // pSE
            if ( mesh->BCp.type[iPrSE] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrSE] - Stokes->neq_mom,   &(nnzc2B[ith]), pSE*celvol, mesh->BCp.type[iPrSE],   mesh->BCp.val[iPrSE],   StokesB->bbc);
            }
        }

        // pE && pW
        if ( mesh->BCp.type[iPrE] != 30 && mesh->BCp.type[c2] != 30 ) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrW] - Stokes->neq_mom, &(nnzc2B[ith]), pW*celvol, mesh->BCp.type[iPrW], mesh->BCp.val[iPrW], StokesB->bbc);
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrE] - Stokes->neq_mom, &(nnzc2B[ith]), pE*celvol, mesh->BCp.type[iPrE], mesh->BCp.val[iPrE], StokesB->bbc);
        }

        if ( Newton==1 && l<nz-1 ) {
            // pNW
            if ( mesh->BCp.type[iPrNW] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrNW] - Stokes->neq_mom,   &(nnzc2B[ith]), pNW*celvol, mesh->BCp.type[iPrNW],   mesh->BCp.val[iPrNW],   StokesB->bbc);
            }

            // pNW
            if ( mesh->BCp.type[iPrNE] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrNE] - Stokes->neq_mom,   &(nnzc2B[ith]), pNE*celvol, mesh->BCp.type[iPrNE],   mesh->BCp.val[iPrNE],   StokesB->bbc);
            }
        }

    }
    else {

        // Residual function
        StokesA->F[eqn] = uC*u[iVxC];
        if ( mesh->BCp.type[iPrSW] != 30 && Newton==1 && l>1   )   StokesA->F[eqn] += pSW*p[iPrSW];
        if ( mesh->BCp.type[iPrSE] != 30 && Newton==1 && l>1   )   StokesA->F[eqn] += pSE*p[iPrSE];
        if ( mesh->BCp.type[iPrW]  != 30 && mesh->BCp.type[iPrE] != 30 ) {
            StokesA->F[eqn]  += pW*p[iPrW] + pE*p[iPrE];
        }
        if ( mesh->BCp.type[iPrNW] != 30 && Newton==1 && l<nz-1)   StokesA->F[eqn] += pNW*p[iPrNW];
        if ( mesh->BCp.type[iPrNE] != 30 && Newton==1 && l<nz-1)   StokesA->F[eqn] += pNE*p[iPrNE];
        //--------------------
        if ( mesh->BCv.type[iVzSE] != 30 )  StokesA->F[eqn] += vSE*v[iVzSE];
        if ( mesh->BCv.type[iVzSW] != 30 )  StokesA->F[eqn] += vSW*v[iVzSW];
        if ( mesh->BCv.type[iVzNE] != 30 )  StokesA->F[eqn] += vNE*v[iVzNE];
        if ( mesh->BCv.type[iVzNW] != 30 )  StokesA->F[eqn] += vNW*v[iVzNW];
        //--------------------
        if ( l>1 && mesh->BCv.type[iVzSSW] != 30 ) StokesA->F[eqn] += vSSW*v[iVzSSW]; // vSSW
        if ( l>1 && mesh->BCv.type[iVzSSE] != 30 ) StokesA->F[eqn] += vSSE*v[iVzSSE]; // vSSE
        //--------------------
        if ( mesh->BCv.type[iVzSWW]    != 30 ) StokesA->F[eqn] += vSWW*v[iVzSWW]; // vSWW (Newton)
        if ( mesh->BCv.type[iVzSEE]    != 30 ) StokesA->F[eqn] += vSEE*v[iVzSEE]; // vSEE (Newton)
        if ( mesh->BCv.type[iVzNWW]    != 30 ) StokesA->F[eqn] += vNWW*v[iVzNWW]; // vNWW (Newton)
        if ( mesh->BCv.type[iVzNEE]    != 30 ) StokesA->F[eqn] += vNEE*v[iVzNEE]; // vNEE (Newton)
        //--------------------
        if ( l<nz-1 && mesh->BCv.type[iVzNNW] != 30 ) StokesA->F[eqn] += vNNW*v[iVzNNW]; // vNNW
        if ( l<nz-1 && mesh->BCv.type[iVzNNE] != 30 ) StokesA->F[eqn] += vNNE*v[iVzNNE]; // vNNE
        //--------------------
        if ( mesh->BCu.type[iVxSW] != 30 && mesh->BCu.type[iVxSW] != 11 ) StokesA->F[eqn] +=      uSW*u[iVxSW];
        if ( mesh->BCu.type[iVxSW] == 11                                ) StokesA->F[eqn] += -2.0*uSW*mesh->BCu.val[iVxSW];
        if ( mesh->BCu.type[iVxS ] != 30 && mesh->BCu.type[iVxS ] != 11 ) StokesA->F[eqn] +=      uS *u[iVxS];
        if ( mesh->BCu.type[iVxS ] == 11                                ) StokesA->F[eqn] += -2.0*uS* mesh->BCu.val[iVxS];
        if ( mesh->BCu.type[iVxSE] != 30 && mesh->BCu.type[iVxSE] != 11 ) StokesA->F[eqn] +=      uSE*u[iVxSE];
        if ( mesh->BCu.type[iVxSE] == 11                                ) StokesA->F[eqn] += -2.0*uSE*mesh->BCu.val[iVxSE];
        //--------------------
        if ( mesh->BCu.type[iVxNW] != 30 && mesh->BCu.type[iVxNW] != 11 ) StokesA->F[eqn] +=      uNW*u[iVxNW];
        if ( mesh->BCu.type[iVxNW] == 11                                ) StokesA->F[eqn] += -2.0*uNW*mesh->BCu.val[iVxNW];
        if ( mesh->BCu.type[iVxN ] != 30 && mesh->BCu.type[iVxN ] != 11 ) StokesA->F[eqn] +=      uN *u[iVxN];
        if ( mesh->BCu.type[iVxN ] == 11                                ) StokesA->F[eqn] += -2.0*uN *mesh->BCu.val[iVxN];
        if ( mesh->BCu.type[iVxNE] != 30 && mesh->BCu.type[iVxNE] != 11 ) StokesA->F[eqn] +=      uNE*u[iVxNE];
        if ( mesh->BCu.type[iVxNE] == 11                                ) StokesA->F[eqn] += -2.0*uNE*mesh->BCu.val[iVxNE];
        //--------------------
        if ( mesh->BCu.type[iVxW]  != 30 ) StokesA->F[eqn] += uW*u[iVxW];
        if ( mesh->BCu.type[iVxE]  != 30 ) StokesA->F[eqn] += uE*u[iVxE];
        //        if ( mesh->BCu.type[c1-nx] == 11 ) StokesA->F[eqn] += -2.0*D33S*one_dz_dz*mesh->BCu.val[c1-nx];
        //        if ( mesh->BCu.type[c1+nx] == 11 ) StokesA->F[eqn] += -2.0*D33N*one_dz_dz*mesh->BCu.val[c1+nx];
        //--------------------
        StokesA->F[eqn] -= (StokesA->b[eqn]);// + Stokes->bbc[eqn];
        StokesA->F[eqn] *= celvol;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void Zjacobian_InnerNodesDecoupled3( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B, int k, int l ) {

    int Newton = 1;

    int iVzC   = c3;
    int iVzW   = iVzC-1,  iVzE   = iVzC+1;
    int iVzS   = c3-nxvz, iVzN   = c3+nxvz;
    int iVzSW  = iVzS-1,  iVzSE  = iVzS+1;
    int iVzNW  = iVzN-1,  iVzNE  = iVzN+1;

    int iVxNW  = c1+nx-1;
    int iVxNE  = c1+nx;
    int iVxSW  = c1-1;
    int iVxSE  = c1;
    int iVxNWW = iVxNW-1;
    int iVxNEE = iVxNE+1;
    int iVxSWW = iVxSW-1;
    int iVxSEE = iVxSE+1;
    int iVxNNW = iVxNW+nx;
    int iVxNNE = iVxNE+nx;
    int iVxSSW = iVxSW-nx;
    int iVxSSE = iVxSE-nx;

    int iPrS  = c2;
    int iPrSW = iPrS-1;
    int iPrSE = iPrS+1;
    int iPrN  = c2+ncx;
    int iPrNW = iPrN-1;
    int iPrNE = iPrN+1;

    int ixyW  = c1-1;
    int ixySW = ixyW-nx;
    int ixyNW = ixyW+nx;
    int ixyE  = c1;
    int ixySE = ixyE-nx;
    int ixyNE = ixyE+nx;

    if (k==1    && mesh->BCv.type[iVzW]==-12) {
        iVxNWW = iVxNW   + (nx-2);
        iVxSWW = iVxSW   + (nx-2);
        iPrNW  = iPrN    + (ncx-1);
        iPrSW  = iPrS    + (ncx-1);
        iVzW   = iVzC    + (nx-2);
        iVzSW  = iVzW    - nxvz;
        iVzNW  = iVzW    + nxvz;
    }
    if (k==nx-1 && mesh->BCv.type[iVzE]==-12) {
        iVxNEE = iVxNE - (nx-2);
        iVxSEE = iVxSE - (nx-2);
        iPrNE  = iPrN  - (ncx-1);
        iPrSE  = iPrS  - (ncx-1);
        iVzE   = iVzC  - (nx-2);
        iVzNE  = iVzE  + nxvz;
        iVzSE  = iVzE  - nxvz;
        ixyE   = ixyE  - (nx-1);
    }

    double  vC=0.0;
    double uSW=0.0, uSE=0.0, uNW=0.0, uNE=0.0,   vS=0.0,   vW=0.0,   vE=0.0,   vN=0.0, pN=0.0, pS=0.0;
    double vSW=0.0, vSE=0.0, vNW=0.0, vNE=0.0, uSSW=0.0, uSSE=0.0, uNNW=0.0, uNNE=0.0;
    double uSWW=0.0, uSEE=0.0, uNWW=0.0, uNEE=0.0;
    double pSW=0.0, pSE=0.0, pNW=0.0, pNE=0.0;

    double D31W  = mesh->D31_s[ixyW];
    double D32W  = mesh->D32_s[ixyW];
    double D33W  = mesh->D33_s[ixyW];
    double D34W  = mesh->D34_s[ixyW];

    double D31E  = mesh->D31_s[ixyE];
    double D32E  = mesh->D32_s[ixyE];
    double D33E  = mesh->D33_s[ixyE];
    double D34E  = mesh->D34_s[ixyE];

    double D21S  = mesh->D21_n[iPrS];
    double D22S  = mesh->D22_n[iPrS];
    double D23S  = mesh->D23_n[iPrS];
    double D24S  = mesh->D24_n[iPrS];

    double D21N  = mesh->D21_n[iPrN];
    double D22N  = mesh->D22_n[iPrN];
    double D23N  = mesh->D23_n[iPrN];
    double D24N  = mesh->D24_n[iPrN];

    //    if ( mesh->BCv.type[iVzE] == 13 ) {D31E = 0.0; D32E = 0.0; D33E = 0.0; D34E = 0.0;}
    //    if ( mesh->BCv.type[iVzW] == 13 ) {D31W = 0.0; D32W = 0.0; D33W = 0.0; D34W = 0.0;}

    double inS=0.0,inN=0.0,inW=0.0, inE = 0.0;

    if (mesh->BCp.type[iPrS] == -1) inS = 1.0;
    if (mesh->BCp.type[iPrN] == -1) inN = 1.0;

    if (mesh->BCg.type[ixyW] != 30 && mesh->BCv.type[iVzW] != 13 ) inW = 1.0;
    if (mesh->BCg.type[ixyE] != 30 && mesh->BCv.type[iVzE] != 13 ) inE = 1.0;

    // XTRA
    double wS=0.0, wN=0.0, wE=0.0, wW=0.0;
    double inSWc=0.0,inSEc=0.0,inNWc=0.0,inNEc=0.0;
    double inSWv=0.0,inSEv=0.0,inNWv=0.0,inNEv=0.0;
    int nzvx = model.Nz+1;
    int nz   = model.Nz;

    //    if (k==1 && mesh->BCv.type[iVzW]==-1) printf("OK west Vz\n");
    //    if (k==nxvz-2 && mesh->BCv.type[iVzE]==-1)  printf("OK east Vz\n");


    if ( (k>1) || (k==1 && mesh->BCv.type[iVzW]==-1) ) {
        if (mesh->BCp.type[iPrSW] == -1) inSWc = 1.0; // modify for periodic ?
        if (mesh->BCp.type[iPrNW] == -1) inNWc = 1.0; // modify for periodic ?
    }

    if ( (k<nxvz-2) || (k==nxvz-2 && mesh->BCv.type[iVzE]==-1) ) {
        if (mesh->BCp.type[iPrSE] == -1) inSEc = 1.0; // modify for periodic ?
        if (mesh->BCp.type[iPrNE] == -1) inNEc = 1.0; // modify for periodic ?
    }

    if (l>0) {
        if( mesh->BCg.type[ixySW] != 30) inSWv = 1.0;
        if( mesh->BCg.type[ixySE] != 30) inSEv = 1.0;
    }

    if (l<nz-1) {
        if( mesh->BCg.type[ixyNW] != 30) inNWv = 1.0;
        if( mesh->BCg.type[ixyNE] != 30) inNEv = 1.0;
    }

    wE = inN + inS + inNEc + inSEc;
    wW = inN + inS + inNWc + inSWc;
    wS = inW + inE + inSWv + inSEv;
    wN = inW + inE + inNWv + inNEv;

    //    if (l==nz-2) {
    //        printf("%f %f %f %f %d %f %f\n", wW,wE,wS,wN, l, inNWc, inSWc);
    //    }

    if (wW>1.0) wW = 1.0/wW;
    if (wE>1.0) wE = 1.0/wE;
    if (wS>1.0) wS = 1.0/wS;
    if (wN>1.0) wN = 1.0/wN;

    //    if (l==1) {
    //        printf("N %f %f \n", wS,wN);
    //
    //    }
    //
    //    if (l==nz-2) {
    //        printf("S %f %f \n", wS,wN);
    //
    //    }
    //
    //    if (k==1) {
    //        printf("W %f %f \n", wS,wN);
    //
    //    }
    //
    //    if (k==nxvz-2) {
    //        printf("E %f %f \n", wS,wN);
    //
    //    }

    double dx = mesh->dx;
    double dz = mesh->dz;

    vW = (1.0/3.0)*inW*(-3*D33W*dz + dx*wW*(inNWc - inSWc)*(D31W*comp + D32W*(comp - 3)) + 3*dx*(D23N*inN*wN - D23S*inS*wS))/(pow(dx, 2)*dz);
    vC = (1.0/3.0)*(-dx*(dx*(D21N*comp*inN + D21S*comp*inS + D22N*inN*(comp - 3) + D22S*inS*(comp - 3)) + 3*dz*(inE - inW)*(-D23N*inN*wN + D23S*inS*wS)) + dz*(dx*(inN - inS)*(-D31E*comp*inE*wE + D31W*comp*inW*wW - D32E*inE*wE*(comp - 3) + D32W*inW*wW*(comp - 3)) + 3*dz*(D33E*inE + D33W*inW)))/(pow(dx, 2)*pow(dz, 2));
    vE = (1.0/3.0)*inE*(-3*D33E*dz - dx*wE*(inNEc - inSEc)*(D31E*comp + D32E*(comp - 3)) + 3*dx*(-D23N*inN*wN + D23S*inS*wS))/(pow(dx, 2)*dz);
    vS = (1.0/3.0)*inS*(-3*D23S*dz*wS*(inSEv - inSWv) + dx*(D21S*comp + D22S*(comp - 3)) + dz*(-D31E*comp*inE*wE + D31W*comp*inW*wW - D32E*inE*wE*(comp - 3) + D32W*inW*wW*(comp - 3)))/(dx*pow(dz, 2));
    vN = (1.0/3.0)*inN*(3*D23N*dz*wN*(inNEv - inNWv) + dx*(D21N*comp + D22N*(comp - 3)) + dz*(D31E*comp*inE*wE - D31W*comp*inW*wW + D32E*inE*wE*(comp - 3) - D32W*inW*wW*(comp - 3)))/(dx*pow(dz, 2));
    uSW = (1.0/3.0)*(dx*(3*dx*(D23N*inN*inW*wN + D23S*inS*wS*(inSWv - inW)) + dz*inS*(D21S*(comp - 3) + D22S*comp)) - dz*(3*D33W*dx*inW + dz*(D31E*inE*inS*wE*(comp - 3) - D31W*inW*wW*(comp - 3)*(inS - inSWc) + D32E*comp*inE*inS*wE - D32W*comp*inW*wW*(inS - inSWc))))/(pow(dx, 2)*pow(dz, 2));
    uSE = (1.0/3.0)*(-dx*(3*dx*(-D23N*inE*inN*wN + D23S*inS*wS*(inE - inSEv)) + dz*inS*(D21S*(comp - 3) + D22S*comp)) + dz*(3*D33E*dx*inE + dz*(D31E*inE*wE*(comp - 3)*(inS - inSEc) - D31W*inS*inW*wW*(comp - 3) + D32E*comp*inE*wE*(inS - inSEc) - D32W*comp*inS*inW*wW)))/(pow(dx, 2)*pow(dz, 2));
    uNW = (1.0/3.0)*(dx*(3*dx*(D23N*inN*wN*(inNWv - inW) + D23S*inS*inW*wS) - dz*inN*(D21N*(comp - 3) + D22N*comp)) + dz*(3*D33W*dx*inW + dz*(-D31E*inE*inN*wE*(comp - 3) + D31W*inW*wW*(comp - 3)*(inN - inNWc) - D32E*comp*inE*inN*wE + D32W*comp*inW*wW*(inN - inNWc))))/(pow(dx, 2)*pow(dz, 2));
    uNE = (1.0/3.0)*(dx*(3*dx*(-D23N*inN*wN*(inE - inNEv) + D23S*inE*inS*wS) + dz*inN*(D21N*(comp - 3) + D22N*comp)) - dz*(3*D33E*dx*inE + dz*(-D31E*inE*wE*(comp - 3)*(inN - inNEc) + D31W*inN*inW*wW*(comp - 3) - D32E*comp*inE*wE*(inN - inNEc) + D32W*comp*inN*inW*wW)))/(pow(dx, 2)*pow(dz, 2));
    vSW = (1.0/3.0)*(-3*D23S*inS*inSWv*wS + D31W*comp*inSWc*inW*wW + D32W*inSWc*inW*wW*(comp - 3))/(dx*dz);
    vSE = (1.0/3.0)*(3*D23S*inS*inSEv*wS - D31E*comp*inE*inSEc*wE - D32E*inE*inSEc*wE*(comp - 3))/(dx*dz);
    vNW = (1.0/3.0)*(3*D23N*inN*inNWv*wN - D31W*comp*inNWc*inW*wW - D32W*inNWc*inW*wW*(comp - 3))/(dx*dz);
    vNE = (1.0/3.0)*(-3*D23N*inN*inNEv*wN + D31E*comp*inE*inNEc*wE + D32E*inE*inNEc*wE*(comp - 3))/(dx*dz);
    uSWW = (1.0/3.0)*inSWc*inW*wW*(D31W*(comp - 3) + D32W*comp)/pow(dx, 2);
    uSEE = (1.0/3.0)*inE*inSEc*wE*(D31E*(comp - 3) + D32E*comp)/pow(dx, 2);
    uNWW = (1.0/3.0)*inNWc*inW*wW*(D31W*(comp - 3) + D32W*comp)/pow(dx, 2);
    uNEE = (1.0/3.0)*inE*inNEc*wE*(D31E*(comp - 3) + D32E*comp)/pow(dx, 2);
    uSSW = -D23S*inS*inSWv*wS/pow(dz, 2);
    uSSE = -D23S*inS*inSEv*wS/pow(dz, 2);
    uNNW = -D23N*inN*inNWv*wN/pow(dz, 2);
    uNNE = -D23N*inN*inNEv*wN/pow(dz, 2);

    pS  = -inS*one_dz + (inS*(D24S*dx + dz*(-D34E*inE*wE + D34W*inW*wW))/(dx*dz));
    pN  =  inN*one_dz + (inN*(-D24N*dx + dz*(-D34E*inE*wE + D34W*inW*wW))/(dx*dz));
    pSW = (D34W*inSWc*inW*wW/dx);
    pSE = (-D34E*inE*inSEc*wE/dx);
    pNW = (D34W*inNWc*inW*wW/dx);
    pNE = (-D34E*inE*inNEc*wE/dx);
    

     // Stabilisation with density gradients
        if (stab==1) {
            double drhodz  = (mesh->rho_n[c2+ncx] - mesh->rho_n[c2])*one_dz;
            double drhodx  = (mesh->rho_s[c1-1  ] - mesh->rho_s[c1])*one_dx;
            double vC_corr = 1.00 * om * model.dt * mesh->gz[c3] * drhodz;
            double vy_corr = 0.25 * om * model.dt * mesh->gz[c3] * drhodx;
            
    //        if (mesh->rho_n[c2+ncx]*1e27<1000) {
    //            drhodz = 3*drhodz;
    ////            printf("mesh->rho_n[c2+ncx] = %2.2e\n", mesh->rho_n[c2+ncx]*1e27);
    ////            printf("mesh->rho_n[c2] = %2.2e\n", mesh->rho_n[c2]*1e27);
    //        }
            // Importante trique, voire meme gigantesque!
            if (vC+vC_corr>0.0) {
                vC  += vC_corr;
//                uSW += vy_corr;
//                uSE += vy_corr;
//                uNW += vy_corr;
//                uNE += vy_corr;
    //            if (vC_corr>1e-6)printf("vC_corr = %2.2e gz=%2.2e\n",vC_corr, mesh->gz[c3]);
    //            printf(mesh->rho_n[c2+ncx])
                      
            }
        }
//    // Stabilisation with density gradients
//    if (stab==1) {
//        double drhodz  = (mesh->rho_n[c2+ncx] - mesh->rho_n[c2])*one_dz;
//        double vC_corr = 1.00 * om * model.dt * mesh->gz[c3] * drhodz;
//        // Importante trique, voire meme gigantesque!
//        if (vC+vC_corr>0.0) vC += vC_corr;
//    }


    // Add contribution from non-conforming Dirichlets
    if ( mesh->BCv.type[iVzW]   == 11 ) vC  -=  vW ;
    if ( mesh->BCv.type[iVzE]   == 11 ) vC  -=  vE ;
    if ( mesh->BCv.type[iVzSW]  == 11 ) vW  -=  vSW;
    if ( mesh->BCv.type[iVzSE]  == 11 ) vE  -=  vSE;
    if ( mesh->BCv.type[iVzNW]  == 11 ) vW  -=  vNW;
    if ( mesh->BCv.type[iVzNE]  == 11 ) vE  -=  vNE;
    if ( mesh->BCu.type[iVxNNW] == 11 ) uNW -= uNNW;
    if ( mesh->BCu.type[iVxNNE] == 11 ) uNE -= uNNE;
    if ( mesh->BCu.type[iVxSSW] == 11 ) uSW -= uSSW;
    if ( mesh->BCu.type[iVxSSE] == 11 ) uSE -= uSSE;

    //    comp = 1.0;
    //    // simpler
    //    vW = -D33W*inW/pow(dx, 2);
    //    vC = -(D22N*inN*((1.0/3.0)*comp/dz - 1/dz) - D22S*inS*(-1.0/3.0*comp/dz + 1.0/dz))/dz - (-D33E*inE/dx - D33W*inW/dx)/dx;
    //    vE = -D33E*inE/pow(dx, 2);
    //    vS =  D22S*inS*((1.0/3.0)*comp/dz - 1/dz)/dz;
    //    vN = -D22N*inN*(-1.0/3.0*comp/dz + 1.0/dz)/dz;
    //    uSW = (1.0/3.0)*D22S*comp*inS/(dx*dz) - D33W*inW/(dx*dz);
    //    uSE = -1.0/3.0*D22S*comp*inS/(dx*dz) + D33E*inE/(dx*dz);
    //    uNW = -1.0/3.0*D22N*comp*inN/(dx*dz) + D33W*inW/(dx*dz);
    //    uNE = (1.0/3.0)*D22N*comp*inN/(dx*dz) - D33E*inE/(dx*dz);
    //
    //    // Pressure gradient
    //    if ( mesh->BCp.type[c2] != 30 && mesh->BCp.type[c2+ncx] != 30 ) {
    //        pS  =  -one_dz;
    //        pN  =   one_dz;
    //    }
    //
    //    if ( mesh->BCv.type[c3-1] == 11 ) vC  -=  vW;
    //    if ( mesh->BCv.type[c3+1] == 11 ) vC  -=  vE;

    if ( Assemble == 1 ) {
        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------

        // uSSW (Newton)
        if ( mesh->BCu.type[iVxSSW] != 30  ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSSW],   &(nnzc2A[ith]), uSSW*celvol, mesh->BCu.type[iVxSSW],    mesh->BCu.val[iVxSSW],    StokesA->bbc );
        }

        // uSSE (Newton)
        if ( mesh->BCu.type[iVxSSE] != 30 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSSE],     &(nnzc2A[ith]), uSSE*celvol, mesh->BCu.type[iVxSSE],    mesh->BCu.val[iVxSSE],    StokesA->bbc );
        }

        // uSWW
        if ( (k>1 ) || (k==1 && mesh->BCv.type[iVxSWW]==-1) ) {
            if( mesh->BCu.type[iVxSWW] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSWW],     &(nnzc2A[ith]), uSWW*celvol, mesh->BCu.type[iVxSWW],    mesh->BCu.val[iVxSWW],    StokesA->bbc );
        }

        // uSW && uSE
        if ( mesh->BCu.type[iVxSW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSW], &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[iVxSW], mesh->BCu.val[iVxSW], StokesA->bbc );
        if ( mesh->BCu.type[iVxSE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSE], &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[iVxSE], mesh->BCu.val[iVxSE], StokesA->bbc );

        // uSEE
        if ( (k<nx-1 ) || (k==nx-1 && mesh->BCv.type[iVxSEE]==-1) ) {
            if( mesh->BCu.type[c1+1] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSEE],     &(nnzc2A[ith]), uSEE*celvol, mesh->BCu.type[iVxSEE],    mesh->BCu.val[iVxSEE],    StokesA->bbc );
        }

        // uNWW
        if ( (k>1    ) || (k==1   && mesh->BCv.type[iVxNWW]==-1) ) {
            if( mesh->BCu.type[c1+nx-2] != 30)  AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNWW],     &(nnzc2A[ith]), uNWW*celvol, mesh->BCu.type[iVxNWW],    mesh->BCu.val[iVxNWW],    StokesA->bbc );
        }

        // uNW && uNE
        if ( mesh->BCu.type[iVxNW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNW], &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[iVxNW], mesh->BCu.val[iVxNW], StokesA->bbc );
        if ( mesh->BCu.type[iVxNE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNE], &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[iVxNE], mesh->BCu.val[iVxNE], StokesA->bbc );

        // uNEE
        if ( (k<nx-1 ) || (k==nx-1 && mesh->BCv.type[iVxNEE]==-1) ) {
            if( mesh->BCu.type[iVxNEE] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNEE],     &(nnzc2A[ith]), uNEE*celvol, mesh->BCu.type[iVxNEE],    mesh->BCu.val[iVxNEE],    StokesA->bbc );
        }

        // uNNW (Newton)
        if ( mesh->BCu.type[iVxNNW] != 30 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNNW],  &(nnzc2A[ith]), uNNW*celvol, mesh->BCu.type[iVxNNW], mesh->BCu.val[iVxNNW], StokesA->bbc );
        }

        // uNNE (Newton)
        if ( mesh->BCu.type[iVxNNE] != 30 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNNE],  &(nnzc2A[ith]), uNNE*celvol, mesh->BCu.type[iVxNNE], mesh->BCu.val[iVxNNE], StokesA->bbc );
        }

        //--------------------

        // vSW (Newton)
        if ( mesh->BCv.type[iVzSW] != 30  ) {
            if      (mesh->BCv.type[iVzSW] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSW],     &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[iVzSW],      mesh->BCv.val[iVzSW],    StokesA->bbc );
            //            else if (mesh->BCv.type[iVzSW] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSW],     &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[iVzSW],    2*mesh->BCv.val[iVzSW],    StokesA->bbc );
        }

        // vS
        if ( mesh->BCv.type[iVzS] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzS],  &(nnzc2A[ith]), vS*celvol, mesh->BCv.type[iVzS], mesh->BCv.val[iVzS], StokesA->bbc );

        // vSE (Newton)
        if ( mesh->BCv.type[iVzSE] != 30  ) {
            if      (mesh->BCv.type[iVzSE] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSE],     &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[iVzSE],      mesh->BCv.val[iVzSE],    StokesA->bbc );
            //            else if (mesh->BCv.type[iVzSE] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSE],     &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[iVzSE],    2*mesh->BCv.val[iVzSE],    StokesA->bbc );
        }

        // vW
        if ( mesh->BCv.type[iVzW] != 30  ) {
            if      (mesh->BCv.type[iVzW] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzW],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[iVzW],    mesh->BCv.val[iVzW],    StokesA->bbc );
            //            else if (mesh->BCv.type[iVzW] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzW],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[iVzW],    2*mesh->BCv.val[iVzW],    StokesA->bbc );
        }

        // vC
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                     &(nnzc2A[ith]), vC*celvol, mesh->BCv.type[iVzC],      mesh->BCv.val[iVzC],      StokesA->bbc );

        // vE
        if ( mesh->BCv.type[iVzE] != 30 ) {
            if      (mesh->BCv.type[iVzE] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzE],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[iVzE],    mesh->BCv.val[iVzE],    StokesA->bbc );
            //            else if (mesh->BCv.type[iVzE] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzE],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[iVzE],    2*mesh->BCv.val[iVzE],    StokesA->bbc );
        }

        // vNW (Newton)
        if ( mesh->BCv.type[iVzNW] != 30  ) {
            if      (mesh->BCv.type[iVzNW] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNW],     &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[iVzNW],      mesh->BCv.val[iVzNW],    StokesA->bbc );
            //            else if (mesh->BCv.type[iVzNW] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNW],     &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[iVzNW],    2*mesh->BCv.val[iVzNW],    StokesA->bbc );
        }

        // vN
        if ( mesh->BCv.type[iVzN] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzN],  &(nnzc2A[ith]), vN*celvol, mesh->BCv.type[iVzN], mesh->BCv.val[iVzN], StokesA->bbc );

        // vNE (Newton)
        if ( mesh->BCv.type[iVzNE] != 30  ) {
            if      (mesh->BCv.type[iVzNE] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNE],     &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[iVzNE],      mesh->BCv.val[iVzNE],    StokesA->bbc );
            //            else if (mesh->BCv.type[iVzNE] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNE],     &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[iVzNE],    2*mesh->BCv.val[iVzNE],    StokesA->bbc );
        }

        //--------------------

        // pSW
        if ( (Newton==1 && k>1) || (Newton==1 && k==1 && mesh->BCp.type[iPrSW]==-1) ) {
            if ( mesh->BCp.type[iPrSW] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrSW] - Stokes->neq_mom,      &(nnzc2B[ith]), pSW*celvol, mesh->BCp.type[iPrSW],     mesh->BCp.val[iPrSW],     StokesB->bbc );
            }
        }

        // pS
        if ( mesh->BCp.type[iPrS] != 30 && mesh->BCp.type[iPrN] != 30) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrS] - Stokes->neq_mom,      &(nnzc2B[ith]), pS*celvol, mesh->BCp.type[iPrS],     mesh->BCp.val[iPrS],     StokesB->bbc );
        }

        // pSE
        if ( (Newton==1 && k<nx-1) || (Newton==1 && k==nx-1 && mesh->BCp.type[iPrSE]==-1) ) {
            if ( mesh->BCp.type[iPrSE] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrSE] - Stokes->neq_mom,      &(nnzc2B[ith]), pSE*celvol, mesh->BCp.type[iPrSE],     mesh->BCp.val[iPrSE],     StokesB->bbc );
            }
        }


        // pNW
        if ( (Newton==1 && k>1) || (Newton==1 && k==1 && mesh->BCp.type[iPrNW]==-1)  ) {
            if ( mesh->BCp.type[iPrNW] != 30) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrNW] - Stokes->neq_mom,  &(nnzc2B[ith]), pNW*celvol, mesh->BCp.type[iPrNW], mesh->BCp.val[iPrNW], StokesB->bbc );
            }
        }

        // pN
        if ( mesh->BCp.type[iPrS] != 30 && mesh->BCp.type[iPrN] != 30) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrN] - Stokes->neq_mom,  &(nnzc2B[ith]), pN*celvol, mesh->BCp.type[iPrN], mesh->BCp.val[c2+ncx], StokesB->bbc );
        }

        if ( (Newton==1 && k<nx-1) || (Newton==1 && k==nx-1 && mesh->BCp.type[iPrNE]==-1) ) {
            // pNE
            if ( mesh->BCp.type[iPrNE] != 30) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrNE] - Stokes->neq_mom,  &(nnzc2B[ith]), pNE*celvol, mesh->BCp.type[iPrNE], mesh->BCp.val[iPrNE], StokesB->bbc );
            }
        }
    }
    else {

        // Residual
        StokesA->F[eqn] = vC*v[iVzC];
        //--------------------
        if ( mesh->BCp.type[iPrSW] != 30 && Newton==1 && k>1    ) StokesA->F[eqn]  += pSW*p[iPrSW];
        if ( mesh->BCp.type[iPrSE] != 30 && Newton==1 && k<nx-1 ) StokesA->F[eqn]  += pSE*p[iPrSE];
        if ( mesh->BCp.type[iPrN] != 30 && mesh->BCp.type[iPrS] != 30 ) {
            StokesA->F[eqn]  += pS*p[iPrS] + pN*p[iPrN];
        }
        if ( mesh->BCp.type[iPrNW] != 30 && Newton==1 && k>1    ) StokesA->F[eqn]  += pNW*p[iPrNW];
        if ( mesh->BCp.type[iPrNE] != 30 && Newton==1 && k<nx-1 ) StokesA->F[eqn]  += pNE*p[iPrNE];
        //--------------------
        if ( mesh->BCu.type[iVxSSW] != 30 && mesh->BCu.type[iVxSSW] != 11 )  StokesA->F[eqn] += uSSW*u[iVxSSW];
        if (  mesh->BCu.type[iVxSSW] == 11 )  StokesA->F[eqn] += 2.0*uSSW*mesh->BCu.type[iVxSSW];
        if ( mesh->BCu.type[iVxSSE] != 30 && mesh->BCu.type[iVxSSE] != 11 )  StokesA->F[eqn] += uSSE*u[iVxSSE];
        if ( mesh->BCu.type[iVxSSE] == 11 )  StokesA->F[eqn] += 2.0*uSSE*mesh->BCu.type[iVxSSE];
        if ( k>1    && mesh->BCu.type[iVxSWW] != 30 )  StokesA->F[eqn] += uSWW*u[iVxSWW];
        if ( k<nx-1 && mesh->BCu.type[iVxSEE] != 30 )  StokesA->F[eqn] += uSEE*u[iVxSEE];
        if ( k>1    && mesh->BCu.type[iVxNWW] != 30 )  StokesA->F[eqn] += uNWW*u[iVxNWW];
        if ( k<nx-1 && mesh->BCu.type[iVxNEE] != 30 )  StokesA->F[eqn] += uNEE*u[iVxNEE];
        if ( mesh->BCu.type[iVxNNW] != 30 && mesh->BCu.type[iVxNNW] != 11 )  StokesA->F[eqn] += uNNW*u[iVxNNW];
        if ( mesh->BCu.type[iVxNNW] == 11 )  StokesA->F[eqn] += 2.0*uNNW*mesh->BCu.type[iVxNNW];
        if ( mesh->BCu.type[iVxNNE] != 30 && mesh->BCu.type[iVxNNE] != 11 )  StokesA->F[eqn] += uNNE*u[iVxNNE];
        if ( mesh->BCu.type[iVxNNE] == 11  )  StokesA->F[eqn] += 2.0*uNNE*mesh->BCu.type[iVxNNE];
        if ( mesh->BCu.type[iVxSW]  != 30 )  StokesA->F[eqn] += uSW*u[iVxSW];
        if ( mesh->BCu.type[iVxSE]  != 30 )  StokesA->F[eqn] += uSE*u[iVxSE];
        if ( mesh->BCu.type[iVxNW]  != 30 )  StokesA->F[eqn] += uNW*u[iVxNW];
        if ( mesh->BCu.type[iVxNE]  != 30 )  StokesA->F[eqn] += uNE*u[iVxNE];
        //--------------------
        if ( mesh->BCv.type[iVzSW] != 30 && mesh->BCv.type[iVzSW] != 11 && mesh->BCv.type[iVzSW] != -12 ) StokesA->F[eqn] += vSW*v[iVzSW];
        if ( mesh->BCv.type[iVzSW] ==  11 ) StokesA->F[eqn] += -2*vSW*mesh->BCv.val[iVzSW];
        if ( mesh->BCv.type[iVzSW] == -12 ) StokesA->F[eqn] +=    vSW*v[c3+nxvz-3-nxvz];
        if ( mesh->BCv.type[iVzW]    != 30 && mesh->BCv.type[iVzW] != 11 && mesh->BCv.type[iVzW] != -12 ) StokesA->F[eqn] += vW*v[iVzW];
        if ( mesh->BCv.type[iVzW] ==  11 ) StokesA->F[eqn] += -2*vW*mesh->BCv.val[iVzW];
        if ( mesh->BCv.type[iVzW] == -12 ) StokesA->F[eqn] += vW*v[c3+nxvz-3];
        if ( mesh->BCv.type[iVzNW]    != 30 && mesh->BCv.type[iVzNW] != 11 && mesh->BCv.type[iVzNW] != -12 ) StokesA->F[eqn] += vNW*v[iVzNW];
        if ( mesh->BCv.type[iVzNW] ==  11 ) StokesA->F[eqn] += -2*vNW*mesh->BCv.val[iVzNW];
        if ( mesh->BCv.type[iVzNW] == -12 ) StokesA->F[eqn] +=    vNW*v[c3+nxvz-3+nxvz];
        //--------------------
        if ( mesh->BCv.type[iVzSE]    != 30 && mesh->BCv.type[iVzSE] != 11 && mesh->BCv.type[iVzSE] != -12 ) StokesA->F[eqn] += vSE*v[iVzSE];
        if ( mesh->BCv.type[iVzSE] ==  11 ) StokesA->F[eqn] += -2*vSE*mesh->BCv.val[iVzSE];
        if ( mesh->BCv.type[iVzSE] == -12 ) StokesA->F[eqn] +=    vSE*v[c3-nxvz+3-nxvz];
        if ( mesh->BCv.type[iVzE]    != 30 && mesh->BCv.type[iVzE] != 11 && mesh->BCv.type[iVzE] != -12 ) StokesA->F[eqn] += vE*v[iVzE];
        if ( mesh->BCv.type[iVzE] ==  11 ) StokesA->F[eqn] += -2*vE*mesh->BCv.val[iVzE];
        if ( mesh->BCv.type[iVzE] == -12 ) StokesA->F[eqn] += vE*v[c3-nxvz+3];
        if ( mesh->BCv.type[iVzNE]    != 30 && mesh->BCv.type[iVzNE] != 11 && mesh->BCv.type[iVzNE] != -12 ) StokesA->F[eqn] += vNE*v[iVzNE];
        if ( mesh->BCv.type[iVzNE] ==  11 ) StokesA->F[eqn] += -2*vNE*mesh->BCv.val[iVzNE];
        if ( mesh->BCv.type[iVzNE] == -12 ) StokesA->F[eqn] +=    vNE*v[c3-nxvz+3+nxvz];
        //--------------------
        if ( mesh->BCv.type[iVzS] != 30 ) StokesA->F[eqn] += vS*v[iVzS];
        if ( mesh->BCv.type[iVzN] != 30 ) StokesA->F[eqn] += vN*v[iVzN];
        //--------------------
        StokesA->F[eqn] -= (StokesA->b[eqn]);
        StokesA->F[eqn] *= celvol;
    }
}
#endif

#if 1

void Xjacobian_InnerNodesDecoupled3( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B, int k, int l  ) {


    int iVxC   = c1;
    int iVxW   = iVxC-1;
    int iVxE   = iVxC+1;
    int iVxS   = iVxC-nx;
    int iVxN   = iVxC+nx;
    int iVxSW  = iVxS-1;
    int iVxSE  = iVxS+1;
    int iVxNW  = iVxN-1;
    int iVxNE  = iVxN+1;
    int iVzSW  = c3-nxvz;
    int iVzSE  = c3-nxvz+1;
    int iVzNW  = c3;
    int iVzNE  = c3+1;
    int iVzSWW = iVzSW-1;
    int iVzSEE = iVzSE+1;
    int iVzNWW = iVzNW-1;
    int iVzNEE = iVzNE+1;
    int iVzNNW = iVzNW+nxvz, iVzSSW = iVzSW-nxvz;
    int iVzNNE = iVzNE+nxvz, iVzSSE = iVzSE-nxvz;

    int iPrW  = c2;
    int iPrE  = c2+1;
    int ixyN  = c1;
    int ixyS  = c1-nx;
    int ixySW = ixyS-1;
    int ixySE = ixyS+1;
    int ixyNW = ixyN-1;
    int ixyNE = ixyN+1;
    int iPrSW = iPrW - ncx;
    int iPrSE = iPrE - ncx;
    int iPrNW = iPrW + ncx;
    int iPrNE = iPrE + ncx;

    // Periodic ends of stencil for inner points
    if (mesh->BCv.type[iVzSWW] == -12)  iVzSWW = iVzSW + (nx-2);
    if (mesh->BCv.type[iVzNWW] == -12)  iVzNWW = iVzNW + (nx-2);
    if (mesh->BCv.type[iVzSEE] == -12)  iVzSEE = iVzSE - (nx-2);
    if (mesh->BCv.type[iVzNEE] == -12)  iVzNEE = iVzNE - (nx-2);

    if (mesh->BCu.type[iVxC] == -2) {
           iVxW   = c1+nx-2;      iPrW = c2+ncx; iVzSW = c3-2; iVzNW = c3+nxvz-2;
           iPrNW  = iPrW+ncx;    iPrSW = iPrW-ncx;
           iVxSW  = iVxW-nx;     iVxNW = iVxW+nx;
           iVzNNW = iVzNW+nxvz; iVzSSW = iVzSW-nxvz;
           iVzSWW = iVzSW-1;    iVzNWW = iVzNW-1;
       }

    int Newton = 1;

    double  uC=0.0;
    double  uS=0.0,  uN=0.0,  uW=0.0,  uE=0.0,  vSW=0.0,  vSE=0.0,  vNW=0.0,  vNE=0.0, pE=0.0, pW=0.0;
    double uSW=0.0, uSE=0.0, uNW=0.0, uNE=0.0, vSWW=0.0, vSEE=0.0, vNWW=0.0, vNEE=0.0, vSSW=0.0, vSSE = 0.0, vNNW=0.0, vNNE=0.0;
    double pSW=0.0, pSE=0.0, pNW=0.0, pNE=0.0;

    double D11E = mesh->D11_n[iPrE];
    double D12E = mesh->D12_n[iPrE];
    double D13E = mesh->D13_n[iPrE];
    double D14E = mesh->D14_n[iPrE];
//    double D15E = mesh->D15_n[iPrE];

    double D11W = mesh->D11_n[iPrW];
    double D12W = mesh->D12_n[iPrW];
    double D13W = mesh->D13_n[iPrW];
    double D14W = mesh->D14_n[iPrW];
//    double D15W = mesh->D15_n[iPrW];

    double D31N = mesh->D31_s[ixyN];
    double D32N = mesh->D32_s[ixyN];
    double D33N = mesh->D33_s[ixyN];
    double D34N = mesh->D34_s[ixyN];
//    double D35N = mesh->D35_s[ixyN];

    double D31S = mesh->D31_s[ixyS];
    double D32S = mesh->D32_s[ixyS];
    double D33S = mesh->D33_s[ixyS];
    double D34S = mesh->D34_s[ixyS];
//    double D35S = mesh->D35_s[ixyS];
//
//
//    if ( mesh->BCu.type[iVxN] == 13 ) {D31N = 0.0; D32N = 0.0; D33N = 0.0; D34N = 0.0;}
//    if ( mesh->BCu.type[iVxS] == 13 ) {D31S = 0.0; D32S = 0.0; D33S = 0.0; D34S = 0.0;}

    double inE=0.0, inW=0.0, inS=0.0, inN = 0.0;

    if (mesh->BCp.type[iPrW] == -1) inW = 1.0;
    if (mesh->BCp.type[iPrE] == -1) inE = 1.0;

    if (mesh->BCg.type[ixyS] != 30 && mesh->BCu.type[iVxS] != 13) inS = 1.0;
    if (mesh->BCg.type[ixyN] != 30 && mesh->BCu.type[iVxN] != 13) inN = 1.0;

    // X-tra
    double wE=0.0, wW=0.0, wS=0.0, wN = 0.0;
    double inSWc=0.0,inSEc=0.0,inNWc=0.0,inNEc=0.0;
    double inSWv=0.0,inSEv=0.0,inNWv=0.0,inNEv=0.0;
    int nzvx = model.Nz+1, nz = model.Nz;

    if (l>1 || (l==1 && mesh->BCu.type[iVxS] == 11 ) ) {
        if (mesh->BCp.type[iPrSW] == -1) inSWc = 1.0;
        if (mesh->BCp.type[iPrSE] == -1) inSEc = 1.0;
    }

    if (l<nzvx-2 || (l==nzvx-2 && mesh->BCu.type[iVxN] == 11 )) {
        if (mesh->BCp.type[iPrNW] == -1) inNWc = 1.0;
        if (mesh->BCp.type[iPrNE] == -1) inNEc = 1.0;
    }

    if ( (k>0)  || (k==0 && (mesh->BCv.type[iVzSW] == -1 || mesh->BCv.type[iVzSW] == 0)) ) {
        if (mesh->BCg.type[ixySW] != 30) inSWv = 1.0;   // modify for periodic
        if (mesh->BCg.type[ixyNW] != 30) inNWv = 1.0;  // modify for periodic
    }
    if ( (k<nx-1) || (k==nx-1 && (mesh->BCv.type[iVzSE] == -1 || mesh->BCv.type[iVzSE] == 0) ) ) {
        if (mesh->BCg.type[ixySE] != 30) inSEv = 1.0;   // modify for periodic
        if (mesh->BCg.type[ixyNE] != 30) inNEv = 1.0;   // modify for periodic
    }
    
    wE = inN + inS + inNEv + inSEv;
    wW = inN + inS + inNWv + inSWv;
    wS = inW + inE + inSWc + inSEc;
    wN = inW + inE + inNWc + inNEc;

    if (wW>1.0) wW = 1.0/wW;
    if (wE>1.0) wE = 1.0/wE;
    if (wS>1.0) wS = 1.0/wS;
    if (wN>1.0) wN = 1.0/wN;

    double dx = mesh->dx;
    double dz = mesh->dz;

    uW = (1.0/3.0)*inW*(-3*D13W*dx*wW*(inNWv - inSWv) + dx*(-D31N*inN*wN*(comp - 3) + D31S*inS*wS*(comp - 3) - D32N*comp*inN*wN + D32S*comp*inS*wS) + dz*(D11W*(comp - 3) + D12W*comp))/(pow(dx, 2)*dz);
    uC = (1.0/3.0)*(dx*(3*dx*(D33N*inN + D33S*inS) + dz*(inE - inW)*(-D31N*inN*wN*(comp - 3) + D31S*inS*wS*(comp - 3) - D32N*comp*inN*wN + D32S*comp*inS*wS)) - dz*(3*dx*(inN - inS)*(-D13E*inE*wE + D13W*inW*wW) + dz*(D11E*inE*(comp - 3) + D11W*inW*(comp - 3) + D12E*comp*inE + D12W*comp*inW)))/(pow(dx, 2)*pow(dz, 2));
    uE = (1.0/3.0)*inE*(3*D13E*dx*wE*(inNEv - inSEv) + dx*(D31N*inN*wN*(comp - 3) - D31S*inS*wS*(comp - 3) + D32N*comp*inN*wN - D32S*comp*inS*wS) + dz*(D11E*(comp - 3) + D12E*comp))/(pow(dx, 2)*dz);
    uS = (1.0/3.0)*inS*(-3*D33S*dx + dz*wS*(inSEc - inSWc)*(D31S*(comp - 3) + D32S*comp) + 3*dz*(D13E*inE*wE - D13W*inW*wW))/(dx*pow(dz, 2));
    uN = (1.0/3.0)*inN*(-3*D33N*dx - dz*wN*(inNEc - inNWc)*(D31N*(comp - 3) + D32N*comp) + 3*dz*(-D13E*inE*wE + D13W*inW*wW))/(dx*pow(dz, 2));
    vSW = (1.0/3.0)*(-dx*(3*D33S*dz*inS + dx*(D31N*comp*inN*inW*wN + D31S*comp*inS*wS*(inSWc - inW) + D32N*inN*inW*wN*(comp - 3) + D32S*inS*wS*(comp - 3)*(inSWc - inW))) + dz*(dx*inW*(D11W*comp + D12W*(comp - 3)) + 3*dz*(D13E*inE*inS*wE - D13W*inW*wW*(inS - inSWv))))/(pow(dx, 2)*pow(dz, 2));
    vSE = (1.0/3.0)*(dx*(3*D33S*dz*inS + dx*(-D31N*comp*inE*inN*wN + D31S*comp*inS*wS*(inE - inSEc) - D32N*inE*inN*wN*(comp - 3) + D32S*inS*wS*(comp - 3)*(inE - inSEc))) - dz*(dx*inE*(D11E*comp + D12E*(comp - 3)) + 3*dz*(D13E*inE*wE*(inS - inSEv) - D13W*inS*inW*wW)))/(pow(dx, 2)*pow(dz, 2));
    vNW = (1.0/3.0)*(dx*(3*D33N*dz*inN - dx*(D31N*comp*inN*wN*(inNWc - inW) + D31S*comp*inS*inW*wS + D32N*inN*wN*(comp - 3)*(inNWc - inW) + D32S*inS*inW*wS*(comp - 3))) - dz*(dx*inW*(D11W*comp + D12W*(comp - 3)) + 3*dz*(-D13E*inE*inN*wE + D13W*inW*wW*(inN - inNWv))))/(pow(dx, 2)*pow(dz, 2));
    vNE = (1.0/3.0)*(-dx*(3*D33N*dz*inN + dx*(-D31N*comp*inN*wN*(inE - inNEc) + D31S*comp*inE*inS*wS - D32N*inN*wN*(comp - 3)*(inE - inNEc) + D32S*inE*inS*wS*(comp - 3))) + dz*(dx*inE*(D11E*comp + D12E*(comp - 3)) + 3*dz*(-D13E*inE*wE*(inN - inNEv) + D13W*inN*inW*wW)))/(pow(dx, 2)*pow(dz, 2));
    uSW = (1.0/3.0)*(-3*D13W*inSWv*inW*wW + D31S*inS*inSWc*wS*(comp - 3) + D32S*comp*inS*inSWc*wS)/(dx*dz);
    uSE = (1.0/3.0)*(3*D13E*inE*inSEv*wE - D31S*inS*inSEc*wS*(comp - 3) - D32S*comp*inS*inSEc*wS)/(dx*dz);
    uNW = (1.0/3.0)*(3*D13W*inNWv*inW*wW - D31N*inN*inNWc*wN*(comp - 3) - D32N*comp*inN*inNWc*wN)/(dx*dz);
    uNE = (1.0/3.0)*(-3*D13E*inE*inNEv*wE + D31N*inN*inNEc*wN*(comp - 3) + D32N*comp*inN*inNEc*wN)/(dx*dz);
    vSWW = -D13W*inSWv*inW*wW/pow(dx, 2);
    vSEE = -D13E*inE*inSEv*wE/pow(dx, 2);
    vNWW = -D13W*inNWv*inW*wW/pow(dx, 2);
    vNEE = -D13E*inE*inNEv*wE/pow(dx, 2);
    vSSW = (1.0/3.0)*inS*inSWc*wS*(D31S*comp + D32S*(comp - 3))/pow(dz, 2);
    vSSE = (1.0/3.0)*inS*inSEc*wS*(D31S*comp + D32S*(comp - 3))/pow(dz, 2);
    vNNW = (1.0/3.0)*inN*inNWc*wN*(D31N*comp + D32N*(comp - 3))/pow(dz, 2);
    vNNE = (1.0/3.0)*inN*inNEc*wN*(D31N*comp + D32N*(comp - 3))/pow(dz, 2);

    pW  =  -inW*one_dx + (inW*( D14W*dz + dx*(-D34N*inN*wN + D34S*inS*wS))/(dx*dz));
    pE  =   inE*one_dx + (inE*(-D14E*dz + dx*(-D34N*inN*wN + D34S*inS*wS))/(dx*dz));
    pSW = (D34S*inS*inSWc*wS/dz);
    pSE = ( D34S*inS*inSEc*wS/dz);
    pNW = (-D34N*inN*inNWc*wN/dz);
    pNE = (-D34N*inN*inNEc*wN/dz);

    // Stabilisation with density gradients
    if (stab==1) {
        double drhodx  = (mesh->rho_n[c2+1] - mesh->rho_n[c2])*one_dx;
        double uC_corr = 1.00 * om * model.dt * mesh->gx[c1] * drhodx;
        // Importante trique, voire meme gigantesque!
        if (uC+uC_corr>0.0) uC += uC_corr;
    }

    // Add contribution from non-conforming Dirichlets
    if ( mesh->BCu.type[iVxS]   == 11 ) uC  -=  uS ;
    if ( mesh->BCu.type[iVxN]   == 11 ) uC  -=  uN ;
    if ( mesh->BCu.type[iVxSW]  == 11 ) uW  -=  uSW;
    if ( mesh->BCu.type[iVxSE]  == 11 ) uE  -=  uSE;
    if ( mesh->BCu.type[iVxNW]  == 11 ) uW  -=  uNW;
    if ( mesh->BCu.type[iVxNE]  == 11 ) uE  -=  uNE;
    if ( mesh->BCv.type[iVzNWW] == 11 ) vNW -= vNWW;
    if ( mesh->BCv.type[iVzNEE] == 11 ) vNE -= vNEE;
    if ( mesh->BCv.type[iVzSWW] == 11 ) vSW -= vSWW;
    if ( mesh->BCv.type[iVzSEE] == 11 ) vSE -= vSEE;


    if ( Assemble == 1 ) {
        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------
        // dsxx/dx - normal stencil

        // uSW (Newton)
        if (mesh->BCu.type[iVxSW] != 30) {
            if (mesh->BCu.type[iVxSW] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSW], &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[iVxSW],     mesh->BCu.val[iVxSW], StokesA->bbc);
            else                             AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSW], &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[iVxSW], 2.0*mesh->BCu.val[iVxSW], StokesA->bbc);
        }

        // uS
        if (mesh->BCu.type[iVxS] != 30) {
            if (mesh->BCu.type[iVxS] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxS], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[iVxS],     mesh->BCu.val[iVxS], StokesA->bbc);
            else                            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxS], &(nnzc2A[ith]), uS*celvol, mesh->BCu.type[iVxS], 2.0*mesh->BCu.val[iVxS], StokesA->bbc);
        }

        // uSE (Newton)
        if (mesh->BCu.type[iVxSE] != 30) {
            if (mesh->BCu.type[iVxSE] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSE], &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[iVxSE],     mesh->BCu.val[iVxSE], StokesA->bbc);
            else                             AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSE], &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[iVxSE], 2.0*mesh->BCu.val[iVxSE], StokesA->bbc);
        }

        // uW
        if (mesh->BCu.type[iVxW]  != 30)     AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxW],  &(nnzc2A[ith]), uW*celvol, mesh->BCu.type[iVxW],  mesh->BCu.val[iVxW],  StokesA->bbc);

        // uC
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                  &(nnzc2A[ith]), uC*celvol, mesh->BCu.type[iVxC],    mesh->BCu.val[iVxC],    StokesA->bbc);

        
//        if (fabs(uC*celvol)>1e6) {
//            printf("Rhooooo uC=%2.2e celvol = %2.2e inS = %2.2e\n", uC , celvol, inS);
//            printf("D11W=%2.2e, D12W=%2.2e, D13W=%2.2e, D14W=%2.2e\n", D11W, D12W, D13W, D14W);
//            printf("D11E=%2.2e, D12E=%2.2e, D13E=%2.2e, D14E=%2.2e\n", D11E, D12E, D13E, D14E);
//            printf("D31S=%2.2e, D32S=%2.2e, D33S=%2.2e, D34S=%2.2e\n", D31S, D32S, D33S, D34S);
//            printf("D31N=%2.2e, D32N=%2.2e, D33N=%2.2e, D34N=%2.2e\n", D31N, D32N, D33N, D34N);
//
//        }
        
        // uE
        if (mesh->BCu.type[iVxE]  != 30)     AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxE],  &(nnzc2A[ith]), uE*celvol, mesh->BCu.type[iVxE],  mesh->BCu.val[iVxE],  StokesA->bbc);

        // uNW (Newton)
        if (mesh->BCu.type[iVxNW] != 30) {
            if (mesh->BCu.type[iVxNW] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNW], &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[iVxNW],     mesh->BCu.val[iVxNW], StokesA->bbc);
            else                             AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNW], &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[iVxNW], 2.0*mesh->BCu.val[iVxNW], StokesA->bbc);
        }

        // uN
        if (mesh->BCu.type[iVxN] != 30) {
            if (mesh->BCu.type[iVxN] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxN], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[iVxN],   mesh->BCu.val[iVxN], StokesA->bbc);
            else                            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxN], &(nnzc2A[ith]), uN*celvol, mesh->BCu.type[iVxN], 2*mesh->BCu.val[iVxN], StokesA->bbc);
        }

        // uNE (Newton)
        if (mesh->BCu.type[iVxNE] != 30) {
            if (mesh->BCu.type[iVxNE] != 11) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNE], &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[iVxNE],     mesh->BCu.val[iVxNE], StokesA->bbc);
            else                             AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNE], &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[iVxNE], 2.0*mesh->BCu.val[iVxNE], StokesA->bbc);
        }

        //--------------------

        // vSSW
        if ( l>1 ) if ( mesh->BCv.type[iVzSSW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSSW],      &(nnzc2A[ith]), vSSW*celvol, mesh->BCv.type[iVzSSW],      mesh->BCv.val[iVzSSW],      StokesA->bbc);

        // vSSE
        if ( l>1 ) if ( mesh->BCv.type[iVzSSE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSSE],      &(nnzc2A[ith]), vSSE*celvol, mesh->BCv.type[iVzSSE],      mesh->BCv.val[iVzSSE],      StokesA->bbc);

        //--------------------


        // vSWW (Newton)
        if ( (k > 1 ) ||  ( (k==0 || k==1) &&  mesh->BCv.type[iVzSWW] == -1) ) {
            if (mesh->BCv.type[iVzSWW] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSWW],   &(nnzc2A[ith]), vSWW*celvol, mesh->BCv.type[iVzSWW],   mesh->BCv.val[iVzSWW],   StokesA->bbc);
        }

        // vSW && vSE
        if ( mesh->BCv.type[iVzSW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSW], &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[iVzSW], mesh->BCv.val[iVzSW],   StokesA->bbc);
        if ( mesh->BCv.type[iVzSE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSE], &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[iVzSE], mesh->BCv.val[iVzSE], StokesA->bbc);

        // vSEE (Newton)
        if ( (k < nx-2 ) ||  ( (k==nx-2 || k==nx-1) &&  mesh->BCv.type[iVzSEE] == -1) ) {
            if ( mesh->BCv.type[iVzSEE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSEE], &(nnzc2A[ith]), vSEE*celvol, mesh->BCv.type[iVzSEE], mesh->BCv.val[iVzSEE], StokesA->bbc);
        }

        // vNWW (Newton)
        if ( (k > 1 ) ||  ( (k==0 || k==1) &&  mesh->BCv.type[iVzNWW] == -1) ) {
            if ( mesh->BCv.type[iVzNWW] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNWW],        &(nnzc2A[ith]), vNWW*celvol, mesh->BCv.type[iVzNWW],        mesh->BCv.val[iVzNWW],        StokesA->bbc);
        }

        // vNW && vNE
        if ( mesh->BCv.type[iVzNW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNW],      &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[iVzNW],      mesh->BCv.val[iVzNW],      StokesA->bbc);
        if ( mesh->BCv.type[iVzNE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNE],      &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[iVzNE],      mesh->BCv.val[iVzNE],      StokesA->bbc);

        // vNEE (Newton)
        if ( (k < nx-2 ) ||  ((k==nx-2 || k==nx-1) &&  mesh->BCv.type[iVzNEE] == -1) ) {
            if ( mesh->BCv.type[iVzNEE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNEE],      &(nnzc2A[ith]), vNEE*celvol, mesh->BCv.type[iVzNEE],      mesh->BCv.val[iVzNEE],      StokesA->bbc);
        }

        //--------------------

        // vNNW
        if ( l<nz-1 ) if ( mesh->BCv.type[iVzNNW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNNW],      &(nnzc2A[ith]), vNNW*celvol, mesh->BCv.type[iVzNNW],      mesh->BCv.val[iVzNNW],      StokesA->bbc);

        // vNNE
        if ( l<nz-1) if ( mesh->BCv.type[iVzNNE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNNE],      &(nnzc2A[ith]), vNNE*celvol, mesh->BCv.type[iVzNNE],      mesh->BCv.val[iVzNNE],      StokesA->bbc);

        //--------------------

        if ( Newton==1 && l>1) {
            // pSW
            if ( mesh->BCp.type[iPrSW] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrSW] - Stokes->neq_mom,   &(nnzc2B[ith]), pSW*celvol, mesh->BCp.type[iPrSW],   mesh->BCp.val[iPrSW],   StokesB->bbc);
            }

            // pSE
            if ( mesh->BCp.type[iPrSE] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrSE] - Stokes->neq_mom,   &(nnzc2B[ith]), pSE*celvol, mesh->BCp.type[iPrSE],   mesh->BCp.val[iPrSE],   StokesB->bbc);
            }
        }

        // pE && pW
        if ( mesh->BCp.type[iPrE] != 30 && mesh->BCp.type[c2] != 30 ) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrW] - Stokes->neq_mom, &(nnzc2B[ith]), pW*celvol, mesh->BCp.type[iPrW], mesh->BCp.val[iPrW], StokesB->bbc);
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrE] - Stokes->neq_mom, &(nnzc2B[ith]), pE*celvol, mesh->BCp.type[iPrE], mesh->BCp.val[iPrE], StokesB->bbc);
        }

        if ( Newton==1 && l<nz-1 ) {
            // pNW
            if ( mesh->BCp.type[iPrNW] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrNW] - Stokes->neq_mom,   &(nnzc2B[ith]), pNW*celvol, mesh->BCp.type[iPrNW],   mesh->BCp.val[iPrNW],   StokesB->bbc);
            }

            // pNW
            if ( mesh->BCp.type[iPrNE] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrNE] - Stokes->neq_mom,   &(nnzc2B[ith]), pNE*celvol, mesh->BCp.type[iPrNE],   mesh->BCp.val[iPrNE],   StokesB->bbc);
            }
        }

    }
    else {
        
//        // Residual function
//        StokesA->F[eqn] = uC*u[iVxC];
//        if ( mesh->BCp.type[iPrSW] != 30 && Newton==1 && l>1   )   StokesA->F[eqn] += pSW*p[iPrSW];
//        if ( mesh->BCp.type[iPrSE] != 30 && Newton==1 && l>1   )   StokesA->F[eqn] += pSE*p[iPrSE];
//        if ( mesh->BCp.type[iPrW]  != 30 && mesh->BCp.type[iPrE] != 30 ) {
//            StokesA->F[eqn]  += pW*p[iPrW] + pE*p[iPrE];
//        }
//        if ( mesh->BCp.type[iPrNW] != 30 && Newton==1 && l<nz-1)   StokesA->F[eqn] += pNW*p[iPrNW];
//        if ( mesh->BCp.type[iPrNE] != 30 && Newton==1 && l<nz-1)   StokesA->F[eqn] += pNE*p[iPrNE];
//        //--------------------
//        if ( mesh->BCv.type[iVzSE] != 30 )  StokesA->F[eqn] += vSE*v[iVzSE];
//        if ( mesh->BCv.type[iVzSW] != 30 )  StokesA->F[eqn] += vSW*v[iVzSW];
//        if ( mesh->BCv.type[iVzNE] != 30 )  StokesA->F[eqn] += vNE*v[iVzNE];
//        if ( mesh->BCv.type[iVzNW] != 30 )  StokesA->F[eqn] += vNW*v[iVzNW];
//        //--------------------
//        if ( l>1 && mesh->BCv.type[iVzSSW] != 30 ) StokesA->F[eqn] += vSSW*v[iVzSSW]; // vSSW
//        if ( l>1 && mesh->BCv.type[iVzSSE] != 30 ) StokesA->F[eqn] += vSSE*v[iVzSSE]; // vSSE
//        //--------------------
//        if ( mesh->BCv.type[iVzSWW]  == -1 ||  mesh->BCv.type[iVzSWW]  == -12 ) StokesA->F[eqn] +=     vSWW*v[iVzSWW];             // vSWW (Newton)
//        if ( mesh->BCv.type[iVzSWW]  == 11 ) StokesA->F[eqn] += 2.0*vSWW*mesh->BCv.val[iVzSWW];
//        if ( mesh->BCv.type[iVzSEE]  == -1 ||  mesh->BCv.type[iVzSEE]  == -12 ) StokesA->F[eqn] +=     vSEE*v[iVzSEE];             // vSEE (Newton)
//        if ( mesh->BCv.type[iVzSEE]  == 11 ) StokesA->F[eqn] += 2.0*vSEE*mesh->BCv.val[iVzSEE];
//        if ( mesh->BCv.type[iVzNWW]  == -1 || mesh->BCv.type[iVzNWW]  == -12 ) StokesA->F[eqn] +=     vNWW*v[iVzNWW];             // vNWW (Newton)
//        if ( mesh->BCv.type[iVzNWW]  == 11 ) StokesA->F[eqn] += 2.0*vNWW*mesh->BCv.val[iVzNWW];
//        if ( mesh->BCv.type[iVzNEE]  == -1 || mesh->BCv.type[iVzNEE]  == -12 ) StokesA->F[eqn] +=     vNEE*v[iVzNEE];             // vNEE (Newton)
//        if ( mesh->BCv.type[iVzNEE]  == 11 ) StokesA->F[eqn] += 2.0*vNEE*mesh->BCv.val[iVzNEE];
//        //--------------------
//        if ( l<nz-1 && mesh->BCv.type[iVzNNW] != 30 ) StokesA->F[eqn] += vNNW*v[iVzNNW]; // vNNW
//        if ( l<nz-1 && mesh->BCv.type[iVzNNE] != 30 ) StokesA->F[eqn] += vNNE*v[iVzNNE]; // vNNE
//        //--------------------
//        if ( mesh->BCu.type[iVxSW] == -1 || mesh->BCu.type[iVxSW] == -2 || mesh->BCu.type[iVxSW] == -12 ) StokesA->F[eqn] +=      uSW*u[iVxSW];
//        if ( mesh->BCu.type[iVxSW] == 11 ) StokesA->F[eqn] +=  2.0*uSW*mesh->BCu.val[iVxSW];
//        if ( mesh->BCu.type[iVxS ] == -1 || mesh->BCu.type[iVxS ] == -2 || mesh->BCu.type[iVxS ] == -12 ) StokesA->F[eqn] +=      uS *u[iVxS];
//        if ( mesh->BCu.type[iVxS ] == 11 ) StokesA->F[eqn] +=  2.0*uS* mesh->BCu.val[iVxS];
//        if ( mesh->BCu.type[iVxSE] == -1 || mesh->BCu.type[iVxSE] == -2 || mesh->BCu.type[iVxSE] == -12 ) StokesA->F[eqn] +=      uSE*u[iVxSE];
//        if ( mesh->BCu.type[iVxSE] == 11 ) StokesA->F[eqn] +=  2.0*uSE*mesh->BCu.val[iVxSE];
//        //--------------------
//        if ( mesh->BCu.type[iVxNW] == -1 || mesh->BCu.type[iVxNW] == -2 || mesh->BCu.type[iVxNW] == -12 ) StokesA->F[eqn] +=      uNW*u[iVxNW];
//        if ( mesh->BCu.type[iVxNW] == 11 ) StokesA->F[eqn] +=  2.0*uNW*mesh->BCu.val[iVxNW];
//        if ( mesh->BCu.type[iVxN ] == -1 || mesh->BCu.type[iVxN ] == -2 || mesh->BCu.type[iVxN ] == -12 ) StokesA->F[eqn] +=      uN *u[iVxN];
//        if ( mesh->BCu.type[iVxN ] == 11 ) StokesA->F[eqn] +=  2.0*uN *mesh->BCu.val[iVxN];
//        if ( mesh->BCu.type[iVxNE] == -1 || mesh->BCu.type[iVxNE] == -2 || mesh->BCu.type[iVxNE] == -12 ) StokesA->F[eqn] +=      uNE*u[iVxNE];
//        if ( mesh->BCu.type[iVxNE] == 11 ) StokesA->F[eqn] +=  2.0*uNE*mesh->BCu.val[iVxNE];
//        //--------------------
//        if ( mesh->BCu.type[iVxW]  != 30 ) StokesA->F[eqn] += uW*u[iVxW];
//        if ( mesh->BCu.type[iVxE]  != 30 ) StokesA->F[eqn] += uE*u[iVxE];
        //--------------------

        StokesA->F[eqn]  = 0.0;
//        StokesA->F[eqn] += (inE*mesh->sxxd[iPrE] - inW*mesh->sxxd[iPrW])/dx;
//        StokesA->F[eqn] -= (inE*mesh->p_in[iPrE] - inW*mesh->p_in[iPrW])/dx;
//        StokesA->F[eqn] += (inN*mesh->sxz[ixyN]  - inS*mesh->sxz[ixyS]) /dz;
        StokesA->F[eqn] += (mesh->sxxd[iPrE] - mesh->sxxd[iPrW])/dx;
        StokesA->F[eqn] -= (mesh->p_in[iPrE] - mesh->p_in[iPrW])/dx;
        StokesA->F[eqn] += (mesh->sxz[ixyN]  - mesh->sxz[ixyS]) /dz;
        StokesA->F[eqn] *= -1.0;

//        StokesA->F[eqn] -= (StokesA->b[eqn]);// + Stokes->bbc[eqn];
        StokesA->F[eqn] -= (StokesA->b[eqn]);
        StokesA->F[eqn] *= celvol;
//        printf("!!!!!!!!!! Fu[%d] = %2.2e\n", eqn, StokesA->F[eqn] );

    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void Zjacobian_InnerNodesDecoupled3( SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, int Assemble, int lev, int stab, int comp, double om, int sign, params model, double one_dx, double one_dz, double one_dx_dx, double one_dz_dz, double one_dx_dz, double celvol, grid* mesh, int ith, int c1, int c2, int c3, int nx, int ncx, int nxvz, int eqn, double* u, double* v, double* p, int **JtempA, double **AtempA, int *nnzc2A, int **JtempB, double **AtempB, int *nnzc2B, int k, int l ) {

    int Newton = 1;
    int periodix = model.isperiodic_x;

    int iVzC   = c3;
    int iVzW   = iVzC-1,  iVzE   = iVzC+1;
    int iVzS   = c3-nxvz, iVzN   = c3+nxvz;
    int iVzSW  = iVzS-1,  iVzSE  = iVzS+1;
    int iVzNW  = iVzN-1,  iVzNE  = iVzN+1;

    int iVxNW  = c1+nx-1;
    int iVxNE  = c1+nx;
    int iVxSW  = c1-1;
    int iVxSE  = c1;
    int iVxNWW = iVxNW-1;
    int iVxNEE = iVxNE+1;
    int iVxSWW = iVxSW-1;
    int iVxSEE = iVxSE+1;
    int iVxNNW = iVxNW+nx;
    int iVxNNE = iVxNE+nx;
    int iVxSSW = iVxSW-nx;
    int iVxSSE = iVxSE-nx;

    int iPrS  = c2;
    int iPrSW = iPrS-1;
    int iPrSE = iPrS+1;
    int iPrN  = c2+ncx;
    int iPrNW = iPrN-1;
    int iPrNE = iPrN+1;

    int ixyW  = c1-1;
    int ixySW = ixyW-nx;
    int ixyNW = ixyW+nx;
    int ixyE  = c1;
    int ixySE = ixyE-nx;
    int ixyNE = ixyE+nx;

    if (k==1    && mesh->BCv.type[iVzW]==-12) {
        iVxNWW = iVxNW   + (nx-2);
        iVxSWW = iVxSW   + (nx-2);
        iPrNW  = iPrN    + (ncx-1);
        iPrSW  = iPrS    + (ncx-1);
        iVzW   = iVzC    + (nx-2); //+ (nxvz-1-1)
        iVzSW  = iVzW    - nxvz;
        iVzNW  = iVzW    + nxvz;
//        ixyW   = ixyW    + (nx-1); // just added
//        printf("periodic left\n");
    }
    if (k==nx-1 && mesh->BCv.type[iVzE]==-12) {
        iVxNEE = iVxNE - (nx-2);
        iVxSEE = iVxSE - (nx-2);
        iPrNE  = iPrN  - (ncx-1);
        iPrSE  = iPrS  - (ncx-1);
        iVzE   = iVzC  - (nx-2);
        iVzNE  = iVzE  + nxvz;
        iVzSE  = iVzE  - nxvz;
//        ixyE   = ixyE  - (nx-1);
//        printf("periodic right\n");
    }

    double  vC=0.0;
    double uSW=0.0, uSE=0.0, uNW=0.0, uNE=0.0,   vS=0.0,   vW=0.0,   vE=0.0,   vN=0.0, pN=0.0, pS=0.0;
    double vSW=0.0, vSE=0.0, vNW=0.0, vNE=0.0, uSSW=0.0, uSSE=0.0, uNNW=0.0, uNNE=0.0;
    double uSWW=0.0, uSEE=0.0, uNWW=0.0, uNEE=0.0;
    double pSW=0.0, pSE=0.0, pNW=0.0, pNE=0.0;

    double D31W  = mesh->D31_s[ixyW];
    double D32W  = mesh->D32_s[ixyW];
    double D33W  = mesh->D33_s[ixyW];
    double D34W  = mesh->D34_s[ixyW];
//    double D35W  = mesh->D35_s[ixyW];

    double D31E  = mesh->D31_s[ixyE];
    double D32E  = mesh->D32_s[ixyE];
    double D33E  = mesh->D33_s[ixyE];
    double D34E  = mesh->D34_s[ixyE];
//    double D35E  = mesh->D35_s[ixyE];

    double D21S  = mesh->D21_n[iPrS];
    double D22S  = mesh->D22_n[iPrS];
    double D23S  = mesh->D23_n[iPrS];
    double D24S  = mesh->D24_n[iPrS];
//    double D25S  = mesh->D25_n[iPrS];

    double D21N  = mesh->D21_n[iPrN];
    double D22N  = mesh->D22_n[iPrN];
    double D23N  = mesh->D23_n[iPrN];
    double D24N  = mesh->D24_n[iPrN];
//    double D25N  = mesh->D25_n[iPrN];
    
    double inS=0.0,inN=0.0,inW=0.0, inE = 0.0;

    if (mesh->BCp.type[iPrS] == -1) inS = 1.0;
    if (mesh->BCp.type[iPrN] == -1) inN = 1.0;

    if (mesh->BCg.type[ixyW] != 30 && mesh->BCv.type[iVzW] != 13 ) inW = 1.0;
    if (mesh->BCg.type[ixyE] != 30 && mesh->BCv.type[iVzE] != 13 ) inE = 1.0;

    // XTRA
    double wS=0.0, wN=0.0, wE=0.0, wW=0.0;
    double inSWc=0.0,inSEc=0.0,inNWc=0.0,inNEc=0.0;
    double inSWv=0.0,inSEv=0.0,inNWv=0.0,inNEv=0.0;
    int nzvx = model.Nz+1;
    int nz   = model.Nz;

    if ( (k>1) || (k==1 && mesh->BCv.type[iVzW]==-1) ) {
        if (mesh->BCp.type[iPrSW] == -1) inSWc = 1.0;
        if (mesh->BCp.type[iPrNW] == -1) inNWc = 1.0;
    }

    if ( (k<nxvz-2) || (k==nxvz-2 && mesh->BCv.type[iVzE]==-1 ) ) {
        if (mesh->BCp.type[iPrSE] == -1) inSEc = 1.0;
        if (mesh->BCp.type[iPrNE] == -1) inNEc = 1.0;
    }

    if (l>0) {
        if( mesh->BCg.type[ixySW] != 30) inSWv = 1.0;
        if( mesh->BCg.type[ixySE] != 30) inSEv = 1.0;
    }

    if (l<nz-1) {
        if( mesh->BCg.type[ixyNW] != 30) inNWv = 1.0;
        if( mesh->BCg.type[ixyNE] != 30) inNEv = 1.0;
    }

    wE = inN + inS + inNEc + inSEc;
    wW = inN + inS + inNWc + inSWc;
    wS = inW + inE + inSWv + inSEv;
    wN = inW + inE + inNWv + inNEv;

    if (wW>1.0) wW = 1.0/wW;
    if (wE>1.0) wE = 1.0/wE;
    if (wS>1.0) wS = 1.0/wS;
    if (wN>1.0) wN = 1.0/wN;

    double dx = mesh->dx;
    double dz = mesh->dz;

    vW = (1.0/3.0)*inW*(-3*D33W*dz + dx*wW*(inNWc - inSWc)*(D31W*comp + D32W*(comp - 3)) + 3*dx*(D23N*inN*wN - D23S*inS*wS))/(pow(dx, 2)*dz);
    vC = (1.0/3.0)*(-dx*(dx*(D21N*comp*inN + D21S*comp*inS + D22N*inN*(comp - 3) + D22S*inS*(comp - 3)) + 3*dz*(inE - inW)*(-D23N*inN*wN + D23S*inS*wS)) + dz*(dx*(inN - inS)*(-D31E*comp*inE*wE + D31W*comp*inW*wW - D32E*inE*wE*(comp - 3) + D32W*inW*wW*(comp - 3)) + 3*dz*(D33E*inE + D33W*inW)))/(pow(dx, 2)*pow(dz, 2));
    vE = (1.0/3.0)*inE*(-3*D33E*dz - dx*wE*(inNEc - inSEc)*(D31E*comp + D32E*(comp - 3)) + 3*dx*(-D23N*inN*wN + D23S*inS*wS))/(pow(dx, 2)*dz);
    vS = (1.0/3.0)*inS*(-3*D23S*dz*wS*(inSEv - inSWv) + dx*(D21S*comp + D22S*(comp - 3)) + dz*(-D31E*comp*inE*wE + D31W*comp*inW*wW - D32E*inE*wE*(comp - 3) + D32W*inW*wW*(comp - 3)))/(dx*pow(dz, 2));
    vN = (1.0/3.0)*inN*(3*D23N*dz*wN*(inNEv - inNWv) + dx*(D21N*comp + D22N*(comp - 3)) + dz*(D31E*comp*inE*wE - D31W*comp*inW*wW + D32E*inE*wE*(comp - 3) - D32W*inW*wW*(comp - 3)))/(dx*pow(dz, 2));
    uSW = (1.0/3.0)*(dx*(3*dx*(D23N*inN*inW*wN + D23S*inS*wS*(inSWv - inW)) + dz*inS*(D21S*(comp - 3) + D22S*comp)) - dz*(3*D33W*dx*inW + dz*(D31E*inE*inS*wE*(comp - 3) - D31W*inW*wW*(comp - 3)*(inS - inSWc) + D32E*comp*inE*inS*wE - D32W*comp*inW*wW*(inS - inSWc))))/(pow(dx, 2)*pow(dz, 2));
    uSE = (1.0/3.0)*(-dx*(3*dx*(-D23N*inE*inN*wN + D23S*inS*wS*(inE - inSEv)) + dz*inS*(D21S*(comp - 3) + D22S*comp)) + dz*(3*D33E*dx*inE + dz*(D31E*inE*wE*(comp - 3)*(inS - inSEc) - D31W*inS*inW*wW*(comp - 3) + D32E*comp*inE*wE*(inS - inSEc) - D32W*comp*inS*inW*wW)))/(pow(dx, 2)*pow(dz, 2));
    uNW = (1.0/3.0)*(dx*(3*dx*(D23N*inN*wN*(inNWv - inW) + D23S*inS*inW*wS) - dz*inN*(D21N*(comp - 3) + D22N*comp)) + dz*(3*D33W*dx*inW + dz*(-D31E*inE*inN*wE*(comp - 3) + D31W*inW*wW*(comp - 3)*(inN - inNWc) - D32E*comp*inE*inN*wE + D32W*comp*inW*wW*(inN - inNWc))))/(pow(dx, 2)*pow(dz, 2));
    uNE = (1.0/3.0)*(dx*(3*dx*(-D23N*inN*wN*(inE - inNEv) + D23S*inE*inS*wS) + dz*inN*(D21N*(comp - 3) + D22N*comp)) - dz*(3*D33E*dx*inE + dz*(-D31E*inE*wE*(comp - 3)*(inN - inNEc) + D31W*inN*inW*wW*(comp - 3) - D32E*comp*inE*wE*(inN - inNEc) + D32W*comp*inN*inW*wW)))/(pow(dx, 2)*pow(dz, 2));
    vSW = (1.0/3.0)*(-3*D23S*inS*inSWv*wS + D31W*comp*inSWc*inW*wW + D32W*inSWc*inW*wW*(comp - 3))/(dx*dz);
    vSE = (1.0/3.0)*(3*D23S*inS*inSEv*wS - D31E*comp*inE*inSEc*wE - D32E*inE*inSEc*wE*(comp - 3))/(dx*dz);
    vNW = (1.0/3.0)*(3*D23N*inN*inNWv*wN - D31W*comp*inNWc*inW*wW - D32W*inNWc*inW*wW*(comp - 3))/(dx*dz);
    vNE = (1.0/3.0)*(-3*D23N*inN*inNEv*wN + D31E*comp*inE*inNEc*wE + D32E*inE*inNEc*wE*(comp - 3))/(dx*dz);
    uSWW = (1.0/3.0)*inSWc*inW*wW*(D31W*(comp - 3) + D32W*comp)/pow(dx, 2);
    uSEE = (1.0/3.0)*inE*inSEc*wE*(D31E*(comp - 3) + D32E*comp)/pow(dx, 2);
    uNWW = (1.0/3.0)*inNWc*inW*wW*(D31W*(comp - 3) + D32W*comp)/pow(dx, 2);
    uNEE = (1.0/3.0)*inE*inNEc*wE*(D31E*(comp - 3) + D32E*comp)/pow(dx, 2);
    uSSW = -D23S*inS*inSWv*wS/pow(dz, 2);
    uSSE = -D23S*inS*inSEv*wS/pow(dz, 2);
    uNNW = -D23N*inN*inNWv*wN/pow(dz, 2);
    uNNE = -D23N*inN*inNEv*wN/pow(dz, 2);
    pS  = -inS*one_dz + (inS*(D24S*dx + dz*(-D34E*inE*wE + D34W*inW*wW))/(dx*dz));
    pN  =  inN*one_dz + (inN*(-D24N*dx + dz*(-D34E*inE*wE + D34W*inW*wW))/(dx*dz));
    pSW = (D34W*inSWc*inW*wW/dx);
    pSE = (-D34E*inE*inSEc*wE/dx);
    pNW = (D34W*inNWc*inW*wW/dx);
    pNE = (-D34E*inE*inNEc*wE/dx);

    // Stabilisation with density gradients
    if (stab==1) {
        double drhodz  = (mesh->rho_n[c2+ncx] - mesh->rho_n[c2])*one_dz;
        double vC_corr = 1.00 * om * model.dt * mesh->gz[c3] * drhodz;
        // Importante trique, voire meme gigantesque!
        if (vC+vC_corr>0.0) vC += vC_corr;
    }

    // Add contribution from non-conforming Dirichlets
    if ( mesh->BCv.type[iVzW]   == 11 ) vC  -=  vW ;
    if ( mesh->BCv.type[iVzE]   == 11 ) vC  -=  vE ;
    if ( mesh->BCv.type[iVzSW]  == 11 ) vW  -=  vSW;
    if ( mesh->BCv.type[iVzSE]  == 11 ) vE  -=  vSE;
    if ( mesh->BCv.type[iVzNW]  == 11 ) vW  -=  vNW;
    if ( mesh->BCv.type[iVzNE]  == 11 ) vE  -=  vNE;
    if ( mesh->BCu.type[iVxNNW] == 11 ) uNW -= uNNW;
    if ( mesh->BCu.type[iVxNNE] == 11 ) uNE -= uNNE;
    if ( mesh->BCu.type[iVxSSW] == 11 ) uSW -= uSSW;
    if ( mesh->BCu.type[iVxSSE] == 11 ) uSE -= uSSE;

    if ( Assemble == 1 ) {
        StokesA->b[eqn] *= celvol;
        StokesB->b[eqn] *= celvol;
        //--------------------

        // uSSW (Newton)
        if ( mesh->BCu.type[iVxSSW]  != 30   ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSSW],   &(nnzc2A[ith]), uSSW*celvol, mesh->BCu.type[iVxSSW],    mesh->BCu.val[iVxSSW],    StokesA->bbc );
        }

        // uSSE (Newton)
        if ( mesh->BCu.type[iVxSSE]   != 30 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSSE],     &(nnzc2A[ith]), uSSE*celvol, mesh->BCu.type[iVxSSE],    mesh->BCu.val[iVxSSE],    StokesA->bbc );
        }
//
        // uSWW
        if ( (k>1 ) || (k==1 && mesh->BCu.type[iVxSWW]==-1) ) {
            if( mesh->BCu.type[iVxSWW] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSWW],     &(nnzc2A[ith]), uSWW*celvol, mesh->BCu.type[iVxSWW],    mesh->BCu.val[iVxSWW],    StokesA->bbc );
        }
//
        // uSW && uSE
        if ( mesh->BCu.type[iVxSW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSW], &(nnzc2A[ith]), uSW*celvol, mesh->BCu.type[iVxSW], mesh->BCu.val[iVxSW], StokesA->bbc );
        if ( mesh->BCu.type[iVxSE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSE], &(nnzc2A[ith]), uSE*celvol, mesh->BCu.type[iVxSE], mesh->BCu.val[iVxSE], StokesA->bbc );
//
        // uSEE
        if ( (k<nx-1 ) || (k==nx-1 && mesh->BCu.type[iVxSEE]==-1) ) {
            if( mesh->BCu.type[c1+1] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxSEE],     &(nnzc2A[ith]), uSEE*celvol, mesh->BCu.type[iVxSEE],    mesh->BCu.val[iVxSEE],    StokesA->bbc );
        }

        // uNWW
        if ( (k>1    ) || (k==1   && mesh->BCu.type[iVxNWW]==-1) ) {
            if( mesh->BCu.type[c1+nx-2] != 30)  AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNWW],     &(nnzc2A[ith]), uNWW*celvol, mesh->BCu.type[iVxNWW],    mesh->BCu.val[iVxNWW],    StokesA->bbc );
        }

        // uNW && uNE
        if ( mesh->BCu.type[iVxNW] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNW], &(nnzc2A[ith]), uNW*celvol, mesh->BCu.type[iVxNW], mesh->BCu.val[iVxNW], StokesA->bbc );
        if ( mesh->BCu.type[iVxNE] != 30 ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNE], &(nnzc2A[ith]), uNE*celvol, mesh->BCu.type[iVxNE], mesh->BCu.val[iVxNE], StokesA->bbc );

        // uNEE
        if ( (k<nx-1 ) || (k==nx-1 && mesh->BCu.type[iVxNEE]==-1) ) {
            if( mesh->BCu.type[iVxNEE] != 30) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNEE],     &(nnzc2A[ith]), uNEE*celvol, mesh->BCu.type[iVxNEE],    mesh->BCu.val[iVxNEE],    StokesA->bbc );
        }

        // uNNW (Newton)
        if ( mesh->BCu.type[iVxNNW]   != 30  ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNNW],  &(nnzc2A[ith]), uNNW*celvol, mesh->BCu.type[iVxNNW], mesh->BCu.val[iVxNNW], StokesA->bbc );
        }

        // uNNE (Newton)
        if ( mesh->BCu.type[iVxNNE] != 30 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_u[iVxNNE],  &(nnzc2A[ith]), uNNE*celvol, mesh->BCu.type[iVxNNE], mesh->BCu.val[iVxNNE], StokesA->bbc );
        }

        //--------------------

        // vSW (Newton)
        if ( mesh->BCv.type[iVzSW] == -1  ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSW],     &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[iVzSW],      mesh->BCv.val[iVzSW],    StokesA->bbc );
//            else if (mesh->BCv.type[iVzSW] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSW],     &(nnzc2A[ith]), vSW*celvol, mesh->BCv.type[iVzSW],    2*mesh->BCv.val[iVzSW],    StokesA->bbc );
        }

        // vS
        if ( mesh->BCv.type[iVzS] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzS],  &(nnzc2A[ith]), vS*celvol, mesh->BCv.type[iVzS], mesh->BCv.val[iVzS], StokesA->bbc );

        // vSE (Newton)
        if ( mesh->BCv.type[iVzSE] == -1  ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSE],     &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[iVzSE],      mesh->BCv.val[iVzSE],    StokesA->bbc );
//            else if (mesh->BCv.type[iVzSE] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzSE],     &(nnzc2A[ith]), vSE*celvol, mesh->BCv.type[iVzSE],    2*mesh->BCv.val[iVzSE],    StokesA->bbc );
        }

        // vW
        if ( mesh->BCv.type[iVzW] == -1  ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzW],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[iVzW],    mesh->BCv.val[iVzW],    StokesA->bbc );
//            else if (mesh->BCv.type[iVzW] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzW],     &(nnzc2A[ith]), vW*celvol, mesh->BCv.type[iVzW],    2*mesh->BCv.val[iVzW],    StokesA->bbc );
        }

        // vC
        AddCoeff2( JtempA[ith], AtempA[ith], eqn, eqn,                     &(nnzc2A[ith]), vC*celvol, mesh->BCv.type[iVzC],      mesh->BCv.val[iVzC],      StokesA->bbc );

        // vE
        if ( mesh->BCv.type[iVzE] == -1 ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzE],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[iVzE],    mesh->BCv.val[iVzE],    StokesA->bbc );
//            else if (mesh->BCv.type[iVzE] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzE],     &(nnzc2A[ith]), vE*celvol, mesh->BCv.type[iVzE],    2*mesh->BCv.val[iVzE],    StokesA->bbc );
        }

        // vNW (Newton)
        if ( mesh->BCv.type[iVzNW] == -1  ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNW],     &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[iVzNW],      mesh->BCv.val[iVzNW],    StokesA->bbc );
//            else if (mesh->BCv.type[iVzNW] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNW],     &(nnzc2A[ith]), vNW*celvol, mesh->BCv.type[iVzNW],    2*mesh->BCv.val[iVzNW],    StokesA->bbc );
        }

        // vN
        if ( mesh->BCv.type[iVzN] != 30  ) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzN],  &(nnzc2A[ith]), vN*celvol, mesh->BCv.type[iVzN], mesh->BCv.val[iVzN], StokesA->bbc );

        // vNE (Newton)
        if ( mesh->BCv.type[iVzNE] == -1  ) {
            AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNE],     &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[iVzNE],      mesh->BCv.val[iVzNE],    StokesA->bbc );
//            else if (mesh->BCv.type[iVzNE] !=-12) AddCoeff2( JtempA[ith], AtempA[ith], eqn, Stokes->eqn_v[iVzNE],     &(nnzc2A[ith]), vNE*celvol, mesh->BCv.type[iVzNE],    2*mesh->BCv.val[iVzNE],    StokesA->bbc );
        }

        //--------------------

        // pSW
        if ( (Newton==1 && k>1) || (Newton==1 && k==1 && periodix==1) ) {
            if ( mesh->BCp.type[iPrSW] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrSW] - Stokes->neq_mom,      &(nnzc2B[ith]), pSW*celvol, mesh->BCp.type[iPrSW],     mesh->BCp.val[iPrSW],     StokesB->bbc );
            }
        }

        // pS
        if ( mesh->BCp.type[iPrS] != 30 && mesh->BCp.type[iPrN] != 30) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrS] - Stokes->neq_mom,      &(nnzc2B[ith]), pS*celvol, mesh->BCp.type[iPrS],     mesh->BCp.val[iPrS],     StokesB->bbc );
        }

        // pSE
        if ( (Newton==1 && k<nx-1) || (Newton==1 && k==nx-1 && periodix==1) ) {
            if ( mesh->BCp.type[iPrSE] != 30 ) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrSE] - Stokes->neq_mom,      &(nnzc2B[ith]), pSE*celvol, mesh->BCp.type[iPrSE],     mesh->BCp.val[iPrSE],     StokesB->bbc );
            }
        }


        // pNW
        if ( (Newton==1 && k>1) || (Newton==1 && k==1 && periodix==1)  ) {
            if ( mesh->BCp.type[iPrNW] != 30) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrNW] - Stokes->neq_mom,  &(nnzc2B[ith]), pNW*celvol, mesh->BCp.type[iPrNW], mesh->BCp.val[iPrNW], StokesB->bbc );
            }
        }

        // pN
        if ( mesh->BCp.type[iPrS] != 30 && mesh->BCp.type[iPrN] != 30) {
            AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrN] - Stokes->neq_mom,  &(nnzc2B[ith]), pN*celvol, mesh->BCp.type[iPrN], mesh->BCp.val[c2+ncx], StokesB->bbc );
        }

        if ( (Newton==1 && k<nx-1) || (Newton==1 && k==nx-1 && periodix==1) ) {
            // pNE
            if ( mesh->BCp.type[iPrNE] != 30) {
                AddCoeff2( JtempB[ith], AtempB[ith], eqn, Stokes->eqn_p[iPrNE] - Stokes->neq_mom,  &(nnzc2B[ith]), pNE*celvol, mesh->BCp.type[iPrNE], mesh->BCp.val[iPrNE], StokesB->bbc );
            }
        }
        
//        // Print values of dof tags
//        if (mesh->BCv.type[iVzC-1]==-12 && l==2) {
//            printf("%d %d %d\n",  mesh->BCv.type[iVzNW], mesh->BCv.type[iVzN], mesh->BCv.type[iVzNE] );
//            printf("%d %d %d\n",  mesh->BCv.type[iVzW], mesh->BCv.type[iVzC], mesh->BCv.type[iVzE] );
//            printf("%d %d %d\n",  mesh->BCv.type[iVzSW], mesh->BCv.type[iVzS], mesh->BCv.type[iVzSE] );
//
//            printf("\n");
//            printf("   %d %d   \n",  mesh->BCu.type[iVxNNW], mesh->BCu.type[iVxNNE]);
//            printf("%d %d %d %d\n",  mesh->BCu.type[iVxNWW], mesh->BCu.type[iVxNW], mesh->BCu.type[iVxNE], mesh->BCu.type[iVxNEE]  );
//            printf("%d %d %d %d\n",  mesh->BCu.type[iVxSWW], mesh->BCu.type[iVxSW], mesh->BCu.type[iVxSE], mesh->BCu.type[iVxSEE]  );
//            printf("   %d %d   \n",  mesh->BCu.type[iVxSSW], mesh->BCu.type[iVxSSE]);
//        }
    }
    else {

//        // Residual
//        StokesA->F[eqn] = vC*v[iVzC];
//        //--------------------
//        if ( mesh->BCp.type[iPrSW] != 30 && Newton==1 && k>1    ) StokesA->F[eqn]  += pSW*p[iPrSW];
//        if ( mesh->BCp.type[iPrSE] != 30 && Newton==1 && k<nx-1 ) StokesA->F[eqn]  += pSE*p[iPrSE];
//        if ( mesh->BCp.type[iPrS  ] != 30 && mesh->BCp.type[iPrN ] != 30 ) {
//            StokesA->F[eqn]  += pS*p[iPrS] + pN*p[iPrN];
//        }
//        if ( mesh->BCp.type[iPrNW] != 30 && Newton==1 && k>1    ) StokesA->F[eqn]  += pNW*p[iPrNW];
//        if ( mesh->BCp.type[iPrNE] != 30 && Newton==1 && k<nx-1 ) StokesA->F[eqn]  += pNE*p[iPrNE];
//        //--------------------
//        if ( mesh->BCu.type[iVxSSW] == -1 )  StokesA->F[eqn] +=     uSSW*u[iVxSSW];
//        if ( mesh->BCu.type[iVxSSW] == 11 )  StokesA->F[eqn] += 2.0*uSSW*mesh->BCu.val[iVxSSW];
//        if ( mesh->BCu.type[iVxSSE] == -1 )  StokesA->F[eqn] +=     uSSE*u[iVxSSE];
//        if ( mesh->BCu.type[iVxSSE] == 11 )  StokesA->F[eqn] += 2.0*uSSE*mesh->BCu.val[iVxSSE];
//        if ( (k>1 )    || (k==1    && mesh->BCu.type[iVxSWW]==-1) ) StokesA->F[eqn] += uSWW*u[iVxSWW];
//        if ( (k<nx-1 ) || (k==nx-1 && mesh->BCu.type[iVxSEE]==-1) ) StokesA->F[eqn] += uSEE*u[iVxSEE];
//        if ( (k>1    ) || (k==1    && mesh->BCu.type[iVxNWW]==-1) ) StokesA->F[eqn] += uNWW*u[iVxNWW];
//        if ( (k<nx-1 ) || (k==nx-1 && mesh->BCu.type[iVxNEE]==-1) ) StokesA->F[eqn] += uNEE*u[iVxNEE];
//        if ( mesh->BCu.type[iVxNNW] == -1 )  StokesA->F[eqn] +=     uNNW*u[iVxNNW];
//        if ( mesh->BCu.type[iVxNNW] == 11 )  StokesA->F[eqn] += 2.0*uNNW*mesh->BCu.val[iVxNNW];
//        if ( mesh->BCu.type[iVxNNE] == -1 )  StokesA->F[eqn] +=     uNNE*u[iVxNNE];
//        if ( mesh->BCu.type[iVxNNE] == 11 )  StokesA->F[eqn] += 2.0*uNNE*mesh->BCu.val[iVxNNE];
//        if ( mesh->BCu.type[iVxSW] != 30 ) StokesA->F[eqn] += uSW*u[iVxSW];
//        if ( mesh->BCu.type[iVxSE] != 30 ) StokesA->F[eqn] += uSE*u[iVxSE];
//        if ( mesh->BCu.type[iVxNW] != 30 ) StokesA->F[eqn] += uNW*u[iVxNW];
//        if ( mesh->BCu.type[iVxNE] != 30 ) StokesA->F[eqn] += uNE*u[iVxNE];
//        //--------------------
//        if ( mesh->BCv.type[iVzSW] == -1  && mesh->BCv.type[iVzSW] != 11 ) StokesA->F[eqn] +=     vSW*v[iVzSW];
//        if (                                    mesh->BCv.type[iVzSW] == 11 ) StokesA->F[eqn] += 2.0*vSW*mesh->BCv.val[iVzSW];
//        if ( mesh->BCv.type[iVzW ] == -1  && mesh->BCv.type[iVzW ] != 11 ) StokesA->F[eqn] +=     vW*v[iVzW];
//        if (                                    mesh->BCv.type[iVzW ] == 11 ) StokesA->F[eqn] += 2.0*vW*mesh->BCv.val[iVzW];
//        if ( mesh->BCv.type[iVzNW] == -1 && mesh->BCv.type[iVzNW] != 11 ) StokesA->F[eqn] +=     vNW*v[iVzNW];
//        if (                                    mesh->BCv.type[iVzNW] == 11 ) StokesA->F[eqn] += 2.0*vNW*mesh->BCv.val[iVzNW];
//        //--------------------
//        if ( mesh->BCv.type[iVzSE] == -1 ) StokesA->F[eqn] +=     vSE*v[iVzSE];
//        if ( mesh->BCv.type[iVzSE] == 11 ) StokesA->F[eqn] += 2.0*vSE*mesh->BCv.val[iVzSE];
//        if ( mesh->BCv.type[iVzE ] == -1 ) StokesA->F[eqn] +=     vE*v[iVzE ];
//        if ( mesh->BCv.type[iVzE ] == 11 ) StokesA->F[eqn] += 2.0*vE*mesh->BCv.val[iVzE ];
//        if ( mesh->BCv.type[iVzNE] == -1 ) StokesA->F[eqn] +=     vNE*v[iVzNE];
//        if ( mesh->BCv.type[iVzNE] == 11 ) StokesA->F[eqn] += 2.0*vNE*mesh->BCv.val[iVzNE];
//        //--------------------
//        if ( mesh->BCv.type[iVzS] != 30 ) StokesA->F[eqn] += vS*v[iVzS];
//        if ( mesh->BCv.type[iVzN] != 30 ) StokesA->F[eqn] += vN*v[iVzN];
//        // --------------------

        StokesA->F[eqn]  = 0.0;
        StokesA->F[eqn] += (mesh->szzd[iPrN] - mesh->szzd[iPrS])/dz;
        StokesA->F[eqn] -= (mesh->p_in[iPrN] - mesh->p_in[iPrS])/dz;
        StokesA->F[eqn] += (mesh->sxz[ixyE]  - mesh->sxz[ixyW]) /dx;
        StokesA->F[eqn] *= -1.0;
        
        StokesA->F[eqn] -= (StokesA->b[eqn]);
        StokesA->F[eqn] *= celvol;
    }
}

#endif


/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

void BuildJacobianOperatorDecoupled( grid *mesh, params model, int lev, double *p_corr, double *p, double *u, double *v, SparseMat *Stokes, SparseMat *StokesA, SparseMat *StokesB, SparseMat *StokesC, SparseMat *StokesD, int Assemble ) {

    // FLAG
    // Assemble=1: Build the linear system of discrete equations
    // Assemble=0: Evaluate Stokes residuals

    int    cc, k, l, c1, c2, c3, nx=mesh->Nx, nz=mesh->Nz, nxvz=nx+1, nzvx=nz+1, ncx=nx-1, ncz=nz-1;

    // Pre-calculate FD coefs
    double celvol    = mesh->dx*mesh->dz;
    double one_dx    = 1.0/mesh->dx;
    double one_dz    = 1.0/mesh->dz;
    double one_dx_dx = 1.0/mesh->dx/mesh->dx;
    double one_dz_dz = 1.0/mesh->dz/mesh->dz;
    double one_dx_dz = 1.0/mesh->dx/mesh->dz;

    // Switches
    int eqn,  sign = 1, comp = 1, stab = 0;
    if (model.free_surf_stab>0) stab = 1;
    if (model.compressible==1  ) comp = 1;
    double theta = model.free_surf_stab;

    // Decompose domain
    int n_th, N, ith;
    int *DD, *estart, *eend, *last_eqn;

    // Temporary parallel matrix
    double **AtempA;
    int **JtempA, **ItempA;
    double **AtempB;
    int **JtempB, **ItempB;
    double **AtempC;
    int **JtempC, **ItempC;
    double **AtempD;
    int **JtempD, **ItempD;

    int nnzA, nnzB, nnzC, nnzD, nnzcA=0, nnzcB=0, nnzcC=0, nnzcD=0;
    int *nnzc2A, *nnzc2B, *nnzc2C, *nnzc2D;

#pragma omp parallel shared(n_th)
    {
        n_th = omp_get_num_threads();
    }
#pragma omp barrier
    
    StokesA->neq = Stokes->neq_mom;
    StokesB->neq = Stokes->neq_mom;
    StokesC->neq = Stokes->neq_cont; StokesC->neq_mom = Stokes->neq_mom;
    StokesD->neq = Stokes->neq_cont; StokesD->neq_mom = Stokes->neq_mom;

    // Matrix initialisation
    if ( Assemble == 1 ) {

        nnzA  = 21*((mesh->Nx-1) * mesh->Nz + (mesh->Nz-1) * mesh->Nx);
        nnzB  = 6*((mesh->Nx-1) * mesh->Nz + (mesh->Nz-1) * mesh->Nx);
        nnzC  = 2*((mesh->Nx-1) * mesh->Nz + (mesh->Nz-1) * mesh->Nx);
        nnzD  = 1*((mesh->Nx-1) * (mesh->Nz-1));
        
        //printf("Assembling  decoupled Jacobian matrix...\n");
        AllocMat( StokesA, nnzA );
        AllocMat( StokesB, nnzB );
        AllocMat( StokesC, nnzC );
        AllocMat( StokesD, nnzD );

    }

    // Build velocity block RHS
    int inc=0;
    for( l=0; l<nzvx; l++) {
        for( k=0; k<nx; k++) {
            cc = k + l*nx;
            if ( mesh->BCu.type[cc] != 0 && mesh->BCu.type[cc] != 30 && mesh->BCu.type[cc] != 11 && mesh->BCu.type[cc] != 13  && mesh->BCu.type[cc] != -12 ) {
                StokesA->b[inc] = mesh->roger_x[cc];
                inc++;
            }
        }
    }
    for( l=0; l<nz; l++) {
        for( k=0; k<nxvz; k++) {
            cc = k + l*nxvz;
            if ( mesh->BCv.type[cc] != 0 && mesh->BCv.type[cc] != 30 && mesh->BCv.type[cc] != 11 && mesh->BCv.type[cc] != 13 && mesh->BCv.type[cc] != -12  ) {
                StokesA->b[inc] = mesh->roger_z[cc];
                inc++;
            }
        }
    }

    // Build pressure block RHS
    inc=0;
    for( l=0; l<ncz; l++) {
        for( k=0; k<ncx; k++) {
            cc = k + l*ncx;
            if ( mesh->BCp.type[cc] != 31 && mesh->BCp.type[cc] != 0 && mesh->BCp.type[cc] != 30 ) {
                StokesC->b[inc] = mesh->rhs_p[cc] ;
                inc++;
            }
        }
    }

    //------------------------------------------------------------------//
    //------------------------- U-momentum -----------------------------//
    //------------------------------------------------------------------//

    // Parallel decomposition
    N = nx*nzvx;
    AllocateDecompositionDecoupled(  n_th, &estart, &eend, &DD, &last_eqn );
    DomainDecompositionDecoupled( N, n_th, estart, eend, DD );

    // Allocate parallel matrix
    if ( Assemble == 1 ) {
        AllocateTempMatArraysDecoupled( &AtempA, &ItempA, &JtempA, n_th, nnzA, Stokes->neq_mom, DD, &nnzc2A  );
        AllocateTempMatArraysDecoupled( &AtempB, &ItempB, &JtempB, n_th, nnzB, Stokes->neq_mom, DD, &nnzc2B  );
    }

#pragma omp parallel shared( eend, estart, mesh, Stokes, StokesA, StokesB, u, v, p_corr, nx, ncx, nzvx, nnzc2A, AtempA, JtempA, ItempA, nnzc2B, AtempB, JtempB, ItempB, last_eqn )  private( ith, l, k, c1, c2, c3, eqn ) firstprivate( model, Assemble, lev, one_dx_dx, one_dz_dz, one_dx_dz, one_dx, one_dz, sign, theta, stab, comp, celvol )
    {
        ith = omp_get_thread_num();

        for( c1=estart[ith]; c1<eend[ith]+1; c1++) {

            k      = mesh->kvx[c1];
            l      = mesh->lvx[c1];
            c2     = k-1 + (l-1)*ncx;
            c3     = k   + l*nxvz;

            // Get equation numbering
            if ( mesh->BCu.type[c1] == -1 || mesh->BCu.type[c1] == 2  || mesh->BCu.type[c1] == -2 ) {
                eqn = Stokes->eqn_u[c1];
                last_eqn[ith]   = eqn;

                if ( Assemble == 1 ) {
                    ItempA[ith][eqn] = nnzc2A[ith];
                    ItempB[ith][eqn] = nnzc2B[ith];
                }

                //--------------------- INNER NODES ---------------------//
                if ( l>0 && l<nzvx-1 && k>0 && k<nx-1 ) {
                     Xjacobian_InnerNodesDecoupled3( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B, k, l );

//                    Xmomentum_InnerNodesDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );

                }

                // Periodic
                if ( k==0  && mesh->BCu.type[c1]==-2  ) {
                    Xjacobian_InnerNodesDecoupled3( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B, k, l );
                }

                // NOT CODED YET
                if ( k==0    && mesh->BCu.type[c1]==2 ) {
                    Xmomentum_WestNeumannDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }

                if ( k==nx-1 && mesh->BCu.type[c1]==2 ) {
                    Xmomentum_EastNeumannDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }
            }
        }
    }

    //-----------------------------------------------------------------//

    if ( Assemble == 1 ) {
        MergeParallelMatrix( StokesA, AtempA, JtempA, ItempA, mesh, estart, eend, &nnzcA, nnzc2A, last_eqn, n_th, mesh->BCu.type, Stokes->eqn_u );
        MergeParallelMatrix( StokesB, AtempB, JtempB, ItempB, mesh, estart, eend, &nnzcB, nnzc2B, last_eqn, n_th, mesh->BCu.type, Stokes->eqn_u );
    }

    if ( Assemble == 1 ) {
        FreeTempMatArraysDecoupled( AtempA, ItempA, JtempA, n_th, nnzc2A );
        FreeTempMatArraysDecoupled( AtempB, ItempB, JtempB, n_th, nnzc2B );
    }
    FreeDecompositionDecoupled( estart, eend, DD, last_eqn );

    //------------------------------------------------------------------//
    //------------------------- V-momentum -----------------------------//
    //------------------------------------------------------------------//

    // Parallel decomposition
    N = nxvz*nz;
    AllocateDecompositionDecoupled(  n_th, &estart, &eend, &DD, &last_eqn );
    DomainDecompositionDecoupled( N, n_th, estart, eend, DD );

    // Allocate parallel matrix
    if ( Assemble == 1 ) {
        AllocateTempMatArraysDecoupled( &AtempA, &ItempA, &JtempA, n_th, nnzA, Stokes->neq_mom, DD, &nnzc2A  );
        AllocateTempMatArraysDecoupled( &AtempB, &ItempB, &JtempB, n_th, nnzB, Stokes->neq_mom, DD, &nnzc2B  );
    }

#pragma omp parallel shared( eend, estart, mesh, Stokes, StokesA, StokesB, u, v, p_corr, nx, ncx, nzvx, nnzc2A, AtempA, JtempA, ItempA, nnzc2B, AtempB, JtempB, ItempB, last_eqn )  private( ith, l, k, c1, c2, c3, eqn ) firstprivate( model, Assemble, lev, one_dx_dx, one_dz_dz, one_dx_dz, one_dx, one_dz, sign, theta, stab, comp, celvol )
    {
        ith = omp_get_thread_num();

        for( c3=estart[ith]; c3<eend[ith]+1; c3++) {

            k  = mesh->kvz[c3];
            l  = mesh->lvz[c3];
            c1 = k   + l*nx;
            c2 = k-1 + (l-1)*ncx;

            // Get equation numbering
            if ( mesh->BCv.type[c3] == -1 || mesh->BCv.type[c3] == 2 ) {
                eqn = Stokes->eqn_v[c3];
                last_eqn[ith]   = eqn;

                if ( Assemble == 1 ) {
                    ItempA[ith][eqn] = nnzc2A[ith];
                    ItempB[ith][eqn] = nnzc2B[ith];
                }

                //--------------------- INNER NODES ---------------------//
                if ( k>0 && k<nxvz-1 && l>0 && l<nz-1 ) {
                    Zjacobian_InnerNodesDecoupled3( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B, k, l );
                }

//                if ( k>0 && k<nxvz-1 && l>0 && l<nz-1 ) {
//
//                                  Zmomentum_InnerNodesDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
//                              }

                //--------------------- INNER NODES ---------------------//

                if ( l==0 && mesh->BCv.type[c3] == 2 ) {
                    Zmomentum_SouthNeumannDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }

                //--------------------- INNER NODES ---------------------//

                if ( l==nz-1 && mesh->BCv.type[c3] == 2 ) {
                    Zmomentum_NorthNeumannDecoupled( Stokes, StokesA, StokesB, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p_corr, JtempA, AtempA, nnzc2A, JtempB, AtempB, nnzc2B );
                }

            }
        }
    }

    //-----------------------------------------------------------------//

    if ( Assemble == 1 ) {
        MergeParallelMatrix( StokesA, AtempA, JtempA, ItempA, mesh, estart, eend, &nnzcA, nnzc2A, last_eqn, n_th, mesh->BCv.type, Stokes->eqn_v );
        MergeParallelMatrix( StokesB, AtempB, JtempB, ItempB, mesh, estart, eend, &nnzcB, nnzc2B, last_eqn, n_th, mesh->BCv.type, Stokes->eqn_v );
    }

    if ( Assemble == 1 ) {
        FreeTempMatArraysDecoupled( AtempA, ItempA, JtempA, n_th, nnzc2A );
        FreeTempMatArraysDecoupled( AtempB, ItempB, JtempB, n_th, nnzc2B );
    }
    FreeDecompositionDecoupled( estart, eend, DD, last_eqn );

    //------------------------------------------------------------------//
    //------------------------- Continuity -----------------------------//
    //------------------------------------------------------------------//

    // Parallel decomposition
    N = ncx*ncz;
    AllocateDecompositionDecoupled(  n_th, &estart, &eend, &DD, &last_eqn );
    DomainDecompositionDecoupled( N, n_th, estart, eend, DD );

    // Allocate parallel matrix
    if ( Assemble == 1 ) {
        AllocateTempMatArraysDecoupled( &AtempC, &ItempC, &JtempC, n_th, nnzC, Stokes->neq_cont, DD, &nnzc2C  );
        AllocateTempMatArraysDecoupled( &AtempD, &ItempD, &JtempD, n_th, nnzD, Stokes->neq_cont, DD, &nnzc2D  );
    }


#pragma omp parallel shared( eend, estart, mesh, Stokes, StokesC, StokesD, u, v, p, nnzc2C, AtempC, JtempC, ItempC, nnzc2D, AtempD, JtempD, ItempD, last_eqn )  private( ith, l, k, c1, c2, c3, eqn, comp ) firstprivate( model, Assemble, lev, one_dx_dx, one_dz_dz, one_dx_dz, one_dx, one_dz, sign, theta, stab, celvol, nx, ncx, nxvz, nzvx )
    {

        ith = omp_get_thread_num();

        for( c2=estart[ith]; c2<eend[ith]+1; c2++) {

            k   = mesh->kp[c2];
            l   = mesh->lp[c2];
            c1  = k   + (l+1)*nx;
            c3  = k   + l*nxvz + 1;

            //--------------------- INNER NODES ---------------------//
            if ( mesh->BCp.type[c2] == -1) {

                comp = mesh->comp_cells[c1];

                eqn = Stokes->eqn_p[c2]  - Stokes->neq_mom;
                last_eqn[ith]   = eqn ;

                if ( Assemble == 1 ) {
                    ItempC[ith][eqn] = nnzc2C[ith];
                    ItempD[ith][eqn] = nnzc2D[ith];
                }
                Continuity_InnerNodesDecoupled( Stokes, StokesC, StokesD, Assemble, lev, stab, comp, theta, sign, model, one_dx, one_dz, one_dx_dx, one_dz_dz, one_dx_dz, celvol, mesh, ith, c1, c2, c3, nx, ncx, nxvz, eqn, u, v, p, JtempC, AtempC, nnzc2C, JtempD, AtempD, nnzc2D, k, l );
            }
        }
    }

    //-----------------------------------------------------------------//

    if ( Assemble == 1 ) {
        MergeParallelMatrixDecoupled( StokesC, AtempC, JtempC, ItempC, mesh, estart, eend, &nnzcC, nnzc2C, last_eqn, n_th, mesh->BCp.type, Stokes->eqn_p );
        MergeParallelMatrixDecoupled( StokesD, AtempD, JtempD, ItempD, mesh, estart, eend, &nnzcD, nnzc2D, last_eqn, n_th, mesh->BCp.type, Stokes->eqn_p );
    }
    if ( Assemble == 1 ) {
        FreeTempMatArraysDecoupled( AtempC, ItempC, JtempC, n_th, nnzc2C );
        FreeTempMatArraysDecoupled( AtempD, ItempD, JtempD, n_th, nnzc2D );
    }
    FreeDecompositionDecoupled( estart, eend, DD, last_eqn );

    //------------------------------------------------------------------//
    //----------------------------- End --------------------------------//
    //------------------------------------------------------------------//

    if ( Assemble == 1 ) {

        // Add contribution from the BC's
        for (k=0; k<StokesA->neq; k++) {
            StokesA->b[k] += StokesA->bbc[k];
            StokesB->b[k] = StokesA->b[k];
        }

        // Add contribution from the BC's
        for (k=0; k<StokesC->neq; k++) {
            StokesC->b[k] += StokesC->bbc[k];
            StokesD->b[k] = StokesC->b[k];
        }

        // Final index
        StokesA->Ic[StokesA->neq] = nnzcA;
        StokesB->Ic[StokesB->neq] = nnzcB;
        StokesC->Ic[StokesC->neq] = nnzcC;
        StokesD->Ic[StokesD->neq] = nnzcD;

        StokesA->nnz = nnzcA;
        StokesB->nnz = nnzcB;
        StokesC->nnz = nnzcC;
        StokesD->nnz = nnzcD;

        // Resize arrays to proper number of non-zeros
        double *bufd;
        int *bufi;
        bufi      = DoodzRealloc(StokesA->J, nnzcA*sizeof(int));
        bufd      = DoodzRealloc(StokesA->A, nnzcA*sizeof(double));
        StokesA->J = bufi;
        StokesA->A = bufd;

        bufi      = DoodzRealloc(StokesB->J, nnzcB*sizeof(int));
        bufd      = DoodzRealloc(StokesB->A, nnzcB*sizeof(double));
        StokesB->J = bufi;
        StokesB->A = bufd;

        bufi      = DoodzRealloc(StokesC->J, nnzcC*sizeof(int));
        bufd      = DoodzRealloc(StokesC->A, nnzcC*sizeof(double));
        StokesC->J = bufi;
        StokesC->A = bufd;

        bufi      = DoodzRealloc(StokesD->J, nnzcD*sizeof(int));
        bufd      = DoodzRealloc(StokesD->A, nnzcD*sizeof(double));
        StokesD->J = bufi;
        StokesD->A = bufd;

//        printf("System size: ndof = %d, nzA = %d nzB = %d nzC = %d nzD = %d\n", Stokes->neq, nnzcA, nnzcB, nnzcC, nnzcD);

//        // Extract Diagonal of A - Viscous block
//        int i, j, locNNZ;
//        int I1, J1;
//
//        if (model.diag_scaling == 1) {
//
//#pragma omp parallel for shared(StokesA) private(I1,J1, i, j, locNNZ )
//            for (i=0;i<StokesA->neq; i++) {
//                I1     = StokesA->Ic[i];
//                locNNZ = StokesA->Ic[i+1] - StokesA->Ic[i];
//                for (J1=0;J1<locNNZ; J1++) {
//                    j = StokesA->J[I1 + J1];
//                    if (i==j) StokesA->d[i] = StokesA->A[I1 + J1];
//                }
//            }
//            // Extract Diagonal of D - Pressure block
//#pragma omp parallel for shared(StokesD) private(i)
//            for (i=0;i<StokesD->neq; i++) {
//                StokesC->d[i] = 1.0;
//            }
//
//            // Scale A
//#pragma omp parallel for shared(StokesA) private(I1,J1, i, j, locNNZ )
//            for (i=0;i<StokesA->neq; i++) {
//
//                StokesA->b[i] *= StokesA->d[i];  // scale RHS
//
//                I1     = StokesA->Ic[i];
//                locNNZ = StokesA->Ic[i+1] - StokesA->Ic[i];
//                for (J1=0;J1<locNNZ; J1++) {
//                    j = StokesA->J[I1 + J1];
//                    StokesA->A[I1 + J1] *= StokesA->d[i]*StokesA->d[j];
//                }
//            }
//
//            // Scale B
//#pragma omp parallel for shared(StokesB,StokesA) private(I1,J1, i, j, locNNZ )
//            for (i=0;i<StokesB->neq; i++) {
//                I1     = StokesB->Ic[i];
//                locNNZ = StokesB->Ic[i+1] - StokesB->Ic[i];
//                for (J1=0;J1<locNNZ; J1++) {
//                    j = StokesB->J[I1 + J1];
//                    StokesB->A[I1 + J1] *= StokesA->d[i]*StokesC->d[j];
//                }
//            }
//
//            // Scale C
//#pragma omp parallel for shared(StokesC,StokesA) private(I1,J1, i, j, locNNZ )
//            for (i=0;i<StokesC->neq; i++) {
//
//                StokesC->b[i] *= StokesC->d[i]; // scale RHS
//
//                I1     = StokesC->Ic[i];
//                locNNZ = StokesC->Ic[i+1] - StokesC->Ic[i];
//                for (J1=0;J1<locNNZ; J1++) {
//                    j = StokesC->J[I1 + J1];
//                    StokesC->A[I1 + J1] *= StokesC->d[i]*StokesA->d[j];
//                }
//            }
//
//        }
//
//        MinMaxArray(StokesA->d, 1, StokesA->neq, "diag. A" );
//
//        printf("\n");
//
//            MinMaxArray(StokesA->A, 1, StokesA->nnz, "A" );
//        printf("\n");


                //--------------------------------------//
#ifndef _VG_
        if ( model.write_debug == 1 ) {

            char *filename;
            asprintf( &filename, "Jacobian_%02dcpu_step%02d_iter%02d.gzip.h5", n_th, model.step, model.nit );
            printf("Writing Jacobian matrix file: %s to disk...\n", filename);
            MinMaxArray(StokesA->F, 1, StokesA->neq, "Fu" );
            MinMaxArray(StokesC->F, 1, StokesC->neq, "Fp" );

            // Fill in DD data structure
            OutputSparseMatrix OutputDDA, OutputDDB, OutputDDC, OutputDDD;

            OutputDDA.V = StokesA->A;
            OutputDDA.Ic = StokesA->Ic;
            OutputDDA.J = StokesA->J;
            OutputDDA.b = StokesA->b;
            OutputDDA.F = StokesA->F;
            OutputDDB.V = StokesB->A;
            OutputDDB.Ic = StokesB->Ic;
            OutputDDB.J = StokesB->J;
            OutputDDB.b = StokesB->b;
            OutputDDC.V = StokesC->A;
            OutputDDC.Ic = StokesC->Ic;
            OutputDDC.J = StokesC->J;
            OutputDDC.b = StokesC->b;
            OutputDDC.F = StokesC->F;
            OutputDDD.V = StokesD->A;
            OutputDDD.Ic = StokesD->Ic;
            OutputDDD.J = StokesD->J;
            OutputDDD.b = StokesD->b;
            OutputDDA.eta_cell = mesh->eta_n;
            OutputDDA.params[0] = nx;
            OutputDDA.params[1] = nz;
            OutputDDA.params[2] = mesh->dx;
            OutputDDA.params[3] = mesh->dz;
            OutputDDA.eqn_u = DoodzMalloc(nx*nzvx*sizeof(int));
            OutputDDA.eqn_v = DoodzMalloc(nxvz*nz*sizeof(int));
            OutputDDA.eqn_p = DoodzMalloc(ncx*ncz*sizeof(int));

            for( l=0; l<nzvx; l++) {
                for( k=0; k<nx; k++) {
                    cc = k + l*nx;
                    c1 = k + l*nx;
                    //                OutputDD.eqn_u[cc]=c1;
                    OutputDDA.eqn_u[cc]=Stokes->eqn_u[cc];
                }
            }
            for( l=0; l<nz; l++) {
                for( k=0; k<nxvz; k++) {
                    cc = k + l*nxvz;
                    c3 = nx*nzvx + k + l*nxvz;
                    //                OutputDD.eqn_v[cc]=c3;
                    OutputDDA.eqn_v[cc]=Stokes->eqn_v[cc];
                }
            }
            for( l=0; l<ncz; l++) {
                for( k=0; k<ncx; k++) {
                    cc = k + l*ncx;
                    c2 = nx*nzvx + nxvz*nz + k + l*ncx;
                    //                OutputDD.eqn_p[cc]=c2;
                    OutputDDA.eqn_p[cc]=Stokes->eqn_p[cc];
                }
            }

            // Send data to file
            create_output_hdf5( filename );
            AddGroup_to_hdf5( filename, "model" );
            AddGroup_to_hdf5( filename, "matrix" );
            AddGroup_to_hdf5( filename, "numbering" );
            AddGroup_to_hdf5( filename, "fields" );
            AddFieldToGroup_generic( _TRUE_, filename, "model", "params" , 'd', 4, OutputDDA.params,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "numbering", "eqn_u" , 'i', nx*nzvx, OutputDDA.eqn_u,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "numbering", "eqn_v" , 'i', nxvz*nz, OutputDDA.eqn_v,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "numbering", "eqn_p" , 'i', ncx*ncz, OutputDDA.eqn_p,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "eta_n" , 'd', ncx*ncz, mesh->eta_n,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "eta_s" , 'd', nx*nz, mesh->eta_s,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "tag_s" , 'c', nx*nz, mesh->BCg.type,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "tag_n" , 'c', ncx*ncz, mesh->BCp.type,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "tag_u" , 'c', nx*nzvx, mesh->BCu.type,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "fields", "tag_v" , 'c', nxvz*nz, mesh->BCv.type,  1 );

            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "IA" , 'i', StokesA->neq+1, OutputDDA.Ic,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "JA" , 'i', nnzcA,  OutputDDA.J,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "VA" , 'd', nnzcA,  OutputDDA.V,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "IB" , 'i', StokesB->neq+1, OutputDDB.Ic,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "JB" , 'i', nnzcB,  OutputDDB.J,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "VB" , 'd', nnzcB,  OutputDDB.V,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "IC" , 'i', StokesC->neq+1, OutputDDC.Ic,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "JC" , 'i', nnzcC,  OutputDDC.J,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "VC" , 'd', nnzcC,  OutputDDC.V,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "ID" , 'i', StokesD->neq+1, OutputDDD.Ic,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "JD" , 'i', nnzcD,  OutputDDD.J,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "VD" , 'd', nnzcD,  OutputDDD.V,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "eta_cell", 'd', ncx*ncz, OutputDDA.eta_cell,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "rhs_mom" , 'd', StokesA->neq, OutputDDA.b,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "rhs_cont", 'd', StokesC->neq, OutputDDC.b,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "F_mom" , 'd', StokesA->neq, OutputDDA.F,  1 );
            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "F_cont", 'd', StokesC->neq, OutputDDC.F,  1 );
            //            AddFieldToGroup_generic( _TRUE_, filename, "matrix", "rhs", 'd', Stokes->neq, OutputDD.b,  1 );

            DoodzFree(OutputDDA.eqn_u);
            DoodzFree(OutputDDA.eqn_v);
            DoodzFree(OutputDDA.eqn_p);
            free(filename);
        }
#endif
    }
}


/*--------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------ M-Doodz -----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

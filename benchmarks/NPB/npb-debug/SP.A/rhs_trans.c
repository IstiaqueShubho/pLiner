//-------------------------------------------------------------------------//
//                                                                         //
//  This benchmark is a serial C version of the NPB SP code. This C        //
//  version is developed by the Center for Manycore Programming at Seoul   //
//  National University and derived from the serial Fortran versions in    //
//  "NPB3.3-SER" developed by NAS.                                         //
//                                                                         //
//  Permission to use, copy, distribute and modify this software for any   //
//  purpose with or without fee is hereby granted. This software is        //
//  provided "as is" without express or implied warranty.                  //
//                                                                         //
//  Information on NPB 3.3, including the technical report, the original   //
//  specifications, source code, results and information on how to submit  //
//  new results, is available at:                                          //
//                                                                         //
//           http://www.nas.nasa.gov/Software/NPB/                         //
//                                                                         //
//  Send comments or suggestions for this C version to cmp@aces.snu.ac.kr  //
//                                                                         //
//          Center for Manycore Programming                                //
//          School of Computer Science and Engineering                     //
//          Seoul National University                                      //
//          Seoul 151-744, Korea                                           //
//                                                                         //
//          E-mail:  cmp@aces.snu.ac.kr                                    //
//                                                                         //
//-------------------------------------------------------------------------//

//-------------------------------------------------------------------------//
// Authors: Sangmin Seo, Jungwon Kim, Jun Lee, Jeongho Nah, Gangwon Jo,    //
//          and Jaejin Lee                                                 //
//-------------------------------------------------------------------------//

#include <math.h>
#include "header.h"

void compute_rhs()
{
  int i, j, k, m;
  long double aux, rho_inv, uijk, up1, um1, vijk, vp1, vm1, wijk, wp1, wm1;


  if (timeron) timer_start(t_rhs);
  //---------------------------------------------------------------------
  // compute the reciprocal of density, and the kinetic energy, 
  // and the speed of sound. 
  //---------------------------------------------------------------------
  for (k = 0; k <= grid_points[2]-1; k++) {
    for (j = 0; j <= grid_points[1]-1; j++) {
      for (i = 0; i <= grid_points[0]-1; i++) {
        rho_inv = 1.0/u[k][j][i][0];
        rho_i[k][j][i] = rho_inv;
        us[k][j][i] = u[k][j][i][1] * rho_inv;
        vs[k][j][i] = u[k][j][i][2] * rho_inv;
        ws[k][j][i] = u[k][j][i][3] * rho_inv;
        square[k][j][i] = 0.5* (
            u[k][j][i][1]*u[k][j][i][1] + 
            u[k][j][i][2]*u[k][j][i][2] +
            u[k][j][i][3]*u[k][j][i][3] ) * rho_inv;
        qs[k][j][i] = square[k][j][i] * rho_inv;
        //-------------------------------------------------------------------
        // (don't need speed and ainx until the lhs computation)
        //-------------------------------------------------------------------
        aux = (long double)c1c2*rho_inv* (u[k][j][i][4] - square[k][j][i]);
        speed[k][j][i] = sqrtl(aux);
      }
    }
  }

  //---------------------------------------------------------------------
  // copy the exact forcing term to the right hand side;  because 
  // this forcing term is known, we can store it on the whole grid
  // including the boundary                   
  //---------------------------------------------------------------------
  for (k = 0; k <= grid_points[2]-1; k++) {
    for (j = 0; j <= grid_points[1]-1; j++) {
      for (i = 0; i <= grid_points[0]-1; i++) {
        for (m = 0; m < 5; m++) {
          rhs[k][j][i][m] = forcing[k][j][i][m];
        }
      }
    }
  }

  //---------------------------------------------------------------------
  // compute xi-direction fluxes 
  //---------------------------------------------------------------------
  if (timeron) timer_start(t_rhsx);
  for (k = 1; k <= nz2; k++) {
    for (j = 1; j <= ny2; j++) {
      for (i = 1; i <= nx2; i++) {
        uijk = us[k][j][i];
        up1  = us[k][j][i+1];
        um1  = us[k][j][i-1];

        rhs[k][j][i][0] = rhs[k][j][i][0] + (long double)dx1tx1 * 
          (u[k][j][i+1][0] - 2.0*u[k][j][i][0] + u[k][j][i-1][0]) -
          (long double)tx2 * (u[k][j][i+1][1] - u[k][j][i-1][1]);

        rhs[k][j][i][1] = rhs[k][j][i][1] + (long double)dx2tx1 * 
          (u[k][j][i+1][1] - 2.0*u[k][j][i][1] + u[k][j][i-1][1]) +
          (long double)xxcon2*(long double)con43 * (up1 - 2.0*uijk + um1) -
          (long double)tx2 * (u[k][j][i+1][1]*up1 - u[k][j][i-1][1]*um1 +
                (u[k][j][i+1][4] - square[k][j][i+1] -
                 u[k][j][i-1][4] + square[k][j][i-1]) * (long double)c2);

        rhs[k][j][i][2] = rhs[k][j][i][2] + (long double)dx3tx1 * 
          (u[k][j][i+1][2] - 2.0*u[k][j][i][2] + u[k][j][i-1][2]) +
          (long double)xxcon2 * (vs[k][j][i+1] - 2.0*vs[k][j][i] + vs[k][j][i-1]) -
          (long double)tx2 * (u[k][j][i+1][2]*up1 - u[k][j][i-1][2]*um1);

        rhs[k][j][i][3] = rhs[k][j][i][3] + (long double)dx4tx1 * 
          (u[k][j][i+1][3] - 2.0*u[k][j][i][3] + u[k][j][i-1][3]) +
          (long double)xxcon2 * (ws[k][j][i+1] - 2.0*ws[k][j][i] + ws[k][j][i-1]) -
          (long double)tx2 * (u[k][j][i+1][3]*up1 - u[k][j][i-1][3]*um1);

        rhs[k][j][i][4] = rhs[k][j][i][4] + (long double)dx5tx1 * 
          (u[k][j][i+1][4] - 2.0*u[k][j][i][4] + u[k][j][i-1][4]) +
          (long double)xxcon3 * (qs[k][j][i+1] - 2.0*qs[k][j][i] + qs[k][j][i-1]) +
          (long double)xxcon4 * (up1*up1 -       2.0*uijk*uijk + um1*um1) +
          (long double)xxcon5 * (u[k][j][i+1][4]*rho_i[k][j][i+1] - 
                2.0*u[k][j][i][4]*rho_i[k][j][i] +
                    u[k][j][i-1][4]*rho_i[k][j][i-1]) -
          (long double)tx2 * ( ((long double)c1*u[k][j][i+1][4] - (long double)c2*square[k][j][i+1])*up1 -
                  ((long double)c1*u[k][j][i-1][4] - (long double)c2*square[k][j][i-1])*um1 );
      }
    }

    //---------------------------------------------------------------------
    // add fourth order xi-direction dissipation               
    //---------------------------------------------------------------------
    for (j = 1; j <= ny2; j++) {
      i = 1;
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m]- (long double)dssp * 
          (5.0*u[k][j][i][m] - 4.0*u[k][j][i+1][m] + u[k][j][i+2][m]);
      }

      i = 2;
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp * 
          (-4.0*u[k][j][i-1][m] + 6.0*u[k][j][i][m] -
            4.0*u[k][j][i+1][m] + u[k][j][i+2][m]);
      }
    }

    for (j = 1; j <= ny2; j++) {
      for (i = 3; i <= nx2-2; i++) {
        for (m = 0; m < 5; m++) {
          rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp * 
            ( u[k][j][i-2][m] - 4.0*u[k][j][i-1][m] + 
            6.0*u[k][j][i][m] - 4.0*u[k][j][i+1][m] + 
              u[k][j][i+2][m] );
        }
      }
    }

    for (j = 1; j <= ny2; j++) {
      i = nx2-1;
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp *
          ( u[k][j][i-2][m] - 4.0*u[k][j][i-1][m] + 
          6.0*u[k][j][i][m] - 4.0*u[k][j][i+1][m] );
      }

      i = nx2;
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp *
          ( u[k][j][i-2][m] - 4.0*u[k][j][i-1][m] + 5.0*u[k][j][i][m] );
      }
    }
  }
  if (timeron) timer_stop(t_rhsx);

  //---------------------------------------------------------------------
  // compute eta-direction fluxes 
  //---------------------------------------------------------------------
  if (timeron) timer_start(t_rhsy);
  for (k = 1; k <= nz2; k++) {
    for (j = 1; j <= ny2; j++) {
      for (i = 1; i <= nx2; i++) {
        vijk = vs[k][j][i];
        vp1  = vs[k][j+1][i];
        vm1  = vs[k][j-1][i];

        rhs[k][j][i][0] = rhs[k][j][i][0] + (long double)dy1ty1 * 
          (u[k][j+1][i][0] - 2.0*u[k][j][i][0] + u[k][j-1][i][0]) -
          (long double)ty2 * (u[k][j+1][i][2] - u[k][j-1][i][2]);

        rhs[k][j][i][1] = rhs[k][j][i][1] + (long double)dy2ty1 * 
          (u[k][j+1][i][1] - 2.0*u[k][j][i][1] + u[k][j-1][i][1]) +
          (long double)yycon2 * (us[k][j+1][i] - 2.0*us[k][j][i] + us[k][j-1][i]) -
          (long double)ty2 * (u[k][j+1][i][1]*vp1 - u[k][j-1][i][1]*vm1);

        rhs[k][j][i][2] = rhs[k][j][i][2] + (long double)dy3ty1 * 
          (u[k][j+1][i][2] - 2.0*u[k][j][i][2] + u[k][j-1][i][2]) +
          (long double)yycon2*(long double)con43 * (vp1 - 2.0*vijk + vm1) -
          (long double)ty2 * (u[k][j+1][i][2]*vp1 - u[k][j-1][i][2]*vm1 +
                (u[k][j+1][i][4] - square[k][j+1][i] - 
                 u[k][j-1][i][4] + square[k][j-1][i]) * (long double)c2);

        rhs[k][j][i][3] = rhs[k][j][i][3] + (long double)dy4ty1 * 
          (u[k][j+1][i][3] - 2.0*u[k][j][i][3] + u[k][j-1][i][3]) +
          (long double)yycon2 * (ws[k][j+1][i] - 2.0*ws[k][j][i] + ws[k][j-1][i]) -
          (long double)ty2 * (u[k][j+1][i][3]*vp1 - u[k][j-1][i][3]*vm1);

        rhs[k][j][i][4] = rhs[k][j][i][4] + (long double)dy5ty1 * 
          (u[k][j+1][i][4] - 2.0*u[k][j][i][4] + u[k][j-1][i][4]) +
          (long double)yycon3 * (qs[k][j+1][i] - 2.0*qs[k][j][i] + qs[k][j-1][i]) +
          (long double)yycon4 * (vp1*vp1       - 2.0*vijk*vijk + vm1*vm1) +
          (long double)yycon5 * (u[k][j+1][i][4]*rho_i[k][j+1][i] - 
                  2.0*u[k][j][i][4]*rho_i[k][j][i] +
                    u[k][j-1][i][4]*rho_i[k][j-1][i]) -
          (long double)ty2 * (((long double)c1*u[k][j+1][i][4] - (long double)c2*square[k][j+1][i]) * vp1 -
                 ((long double)c1*u[k][j-1][i][4] - (long double)c2*square[k][j-1][i]) * vm1);
      }
    }

    //---------------------------------------------------------------------
    // add fourth order eta-direction dissipation         
    //---------------------------------------------------------------------
    j = 1;
    for (i = 1; i <= nx2; i++) {
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m]- (long double)dssp * 
          ( 5.0*u[k][j][i][m] - 4.0*u[k][j+1][i][m] + u[k][j+2][i][m]);
      }
    }

    j = 2;
    for (i = 1; i <= nx2; i++) {
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp * 
          (-4.0*u[k][j-1][i][m] + 6.0*u[k][j][i][m] -
            4.0*u[k][j+1][i][m] + u[k][j+2][i][m]);
      }
    }

    for (j = 3; j <= ny2-2; j++) {
      for (i = 1; i <= nx2; i++) {
        for (m = 0; m < 5; m++) {
          rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp * 
            ( u[k][j-2][i][m] - 4.0*u[k][j-1][i][m] + 
            6.0*u[k][j][i][m] - 4.0*u[k][j+1][i][m] + 
              u[k][j+2][i][m] );
        }
      }
    }

    j = ny2-1;
    for (i = 1; i <= nx2; i++) {
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp *
          ( u[k][j-2][i][m] - 4.0*u[k][j-1][i][m] + 
          6.0*u[k][j][i][m] - 4.0*u[k][j+1][i][m] );
      }
    }

    j = ny2;
    for (i = 1; i <= nx2; i++) {
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp *
          ( u[k][j-2][i][m] - 4.0*u[k][j-1][i][m] + 5.0*u[k][j][i][m] );
      }
    }
  }
  if (timeron) timer_stop(t_rhsy);

  //---------------------------------------------------------------------
  // compute zeta-direction fluxes 
  //---------------------------------------------------------------------
  if (timeron) timer_start(t_rhsz);
  for (k = 1; k <= nz2; k++) {
    for (j = 1; j <= ny2; j++) {
      for (i = 1; i <= nx2; i++) {
        wijk = ws[k][j][i];
        wp1  = ws[k+1][j][i];
        wm1  = ws[k-1][j][i];

        rhs[k][j][i][0] = rhs[k][j][i][0] + (long double)dz1tz1 * 
          (u[k+1][j][i][0] - 2.0*u[k][j][i][0] + u[k-1][j][i][0]) -
          (long double)tz2 * (u[k+1][j][i][3] - u[k-1][j][i][3]);

        rhs[k][j][i][1] = rhs[k][j][i][1] + (long double)dz2tz1 * 
          (u[k+1][j][i][1] - 2.0*u[k][j][i][1] + u[k-1][j][i][1]) +
          (long double)zzcon2 * (us[k+1][j][i] - 2.0*us[k][j][i] + us[k-1][j][i]) -
          (long double)tz2 * (u[k+1][j][i][1]*wp1 - u[k-1][j][i][1]*wm1);

        rhs[k][j][i][2] = rhs[k][j][i][2] + (long double)dz3tz1 * 
          (u[k+1][j][i][2] - 2.0*u[k][j][i][2] + u[k-1][j][i][2]) +
          (long double)zzcon2 * (vs[k+1][j][i] - 2.0*vs[k][j][i] + vs[k-1][j][i]) -
          (long double)tz2 * (u[k+1][j][i][2]*wp1 - u[k-1][j][i][2]*wm1);

        rhs[k][j][i][3] = rhs[k][j][i][3] + (long double)dz4tz1 * 
          (u[k+1][j][i][3] - 2.0*u[k][j][i][3] + u[k-1][j][i][3]) +
          (long double)zzcon2*(long double)con43 * (wp1 - 2.0*wijk + wm1) -
          (long double)tz2 * (u[k+1][j][i][3]*wp1 - u[k-1][j][i][3]*wm1 +
                (u[k+1][j][i][4] - square[k+1][j][i] - 
                 u[k-1][j][i][4] + square[k-1][j][i]) * (long double)c2);

        rhs[k][j][i][4] = rhs[k][j][i][4] + (long double)dz5tz1 * 
          (u[k+1][j][i][4] - 2.0*u[k][j][i][4] + u[k-1][j][i][4]) +
          (long double)zzcon3 * (qs[k+1][j][i] - 2.0*qs[k][j][i] + qs[k-1][j][i]) +
          (long double)zzcon4 * (wp1*wp1 - 2.0*wijk*wijk + wm1*wm1) +
          (long double)zzcon5 * (u[k+1][j][i][4]*rho_i[k+1][j][i] - 
                  2.0*u[k][j][i][4]*rho_i[k][j][i] +
                    u[k-1][j][i][4]*rho_i[k-1][j][i]) -
          (long double)tz2 * (((long double)c1*u[k+1][j][i][4] - (long double)c2*square[k+1][j][i])*wp1 -
                 ((long double)c1*u[k-1][j][i][4] - (long double)c2*square[k-1][j][i])*wm1);
      }
    }
  }

  //---------------------------------------------------------------------
  // add fourth order zeta-direction dissipation                
  //---------------------------------------------------------------------
  k = 1;
  for (j = 1; j <= ny2; j++) {
    for (i = 1; i <= nx2; i++) {
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m]- (long double)dssp * 
          (5.0*u[k][j][i][m] - 4.0*u[k+1][j][i][m] + u[k+2][j][i][m]);
      }
    }
  }

  k = 2;
  for (j = 1; j <= ny2; j++) {
    for (i = 1; i <= nx2; i++) {
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp * 
          (-4.0*u[k-1][j][i][m] + 6.0*u[k][j][i][m] -
            4.0*u[k+1][j][i][m] + u[k+2][j][i][m]);
      }
    }
  }

  for (k = 3; k <= nz2-2; k++) {
    for (j = 1; j <= ny2; j++) {
      for (i = 1; i <= nx2; i++) {
        for (m = 0; m < 5; m++) {
          rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp * 
            ( u[k-2][j][i][m] - 4.0*u[k-1][j][i][m] + 
            6.0*u[k][j][i][m] - 4.0*u[k+1][j][i][m] + 
              u[k+2][j][i][m] );
        }
      }
    }
  }

  k = nz2-1;
  for (j = 1; j <= ny2; j++) {
    for (i = 1; i <= nx2; i++) {
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp *
          ( u[k-2][j][i][m] - 4.0*u[k-1][j][i][m] + 
          6.0*u[k][j][i][m] - 4.0*u[k+1][j][i][m] );
      }
    }
  }

  k = nz2;
  for (j = 1; j <= ny2; j++) {
    for (i = 1; i <= nx2; i++) {
      for (m = 0; m < 5; m++) {
        rhs[k][j][i][m] = rhs[k][j][i][m] - (long double)dssp *
          ( u[k-2][j][i][m] - 4.0*u[k-1][j][i][m] + 5.0*u[k][j][i][m] );
      }
    }
  }
  if (timeron) timer_stop(t_rhsz);

  for (k = 1; k <= nz2; k++) {
    for (j = 1; j <= ny2; j++) {
      for (i = 1; i <= nx2; i++) {
        for (m = 0; m < 5; m++) {
          rhs[k][j][i][m] = rhs[k][j][i][m] * (long double)dt;
        }
      }
    }
  }
  if (timeron) timer_stop(t_rhs);
}

/*
*  Copyright (C) 2007 Xavier Siemens
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with with program; see the file COPYING. If not, write to the
*  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*/

/************************************ <lalVerbatim file="LALDemodFASTCV">
Author: Berukoff, S.J.,  Papa, M.A., Allen, B., Siemens, X $Id$
************************************* </lalVerbatim> */

/* <lalLaTeX>
\subsection{Module \texttt{LALDemodFAST.c}}\label{ss:LALDemodFAST.c}
Computes a demodulated Fourier transform (DeFT) given a set of input
short Fourier transforms (SFT).

\subsubsection*{Prototypes}
\vspace{0.1in}
\input{LALDemodCP}
\idx{LALDemod()}

\subsubsection*{Description}

This routine computes the $F$ statistic for a set of templates that
are defined by: one sky position, a set of spin-down parameters and a
band of possible signal frequencies. The $F$ statistic is described in
JKS, Phys Rev D 58, 063001 (1998). Here, it has been adapted to a
single emission frequency model.

The {\bf parameter structure} defines the search frequency band
(\verb@f0@ and \verb@imax@), the search frequency resolution
(\verb@df@) the first frequency of the input SFTs (\verb@ifmin@), how
many SFTs have to be combined (\verb@SFTno@) and template parameters
(\verb@*spinDwnOrder@, \verb@*spinDwn@ and
@\verb@*skyConst@). \verb@amcoe@ contains the values of the amplitude
modulation functions $a$ and $b$.  \verb@Dterms@ represents the
numbers of terms to be summed to compute the Dirichlet kernel on each
side of the instantaneous frequency.

The {\bf input} is: \verb@**input@, an array of structures of type
\verb@FFT@. This data type will soon disappear as it is just a
complex8frequencyseries.

The {\bf output} is a pointer a structure of type \verb+LALFstat+
containing an array of the values of $\mathcal{F}$. In addition, if
\verb+DemodPar->returnFaFb == TRUE+, the values of $F_a$ and $F_b$
will be returned in addition.  (Memory has to be allocated correctly
beforehand!)

\subsubsection*{Algorithm}

The routine implements the analytical result of eq. \ref{DeFT_algo}.
It thus uses a nested-loop structure, which computes $F$ for all the
template frequencies.

The outer most loop is over the search frequencies.  The next loop is
over $\alpha$, which identifies the SFTs.  The value of $k^*$ is then
computed using the second of Eq. \ref{DeFT_defs}, and thus the
summation over $k$ of Eq.\ref{DeFT_algo} is carried out, with a loop
over Dterms.  In this loop the product $\tilde{x}_{\alpha k}P_{\alpha
k}$ is calculated.  Once this loop completes, $e^{iy_\alpha}$ is
computed, the summation over $\alpha$ performed and, finally, the code
yields the DeFT $\hat{x}_b$.  It can be seen that the code closely
follows the analytical development of the formalism.

Finally, note that in order to avoid repeated trigonometric function
computations, a look-up-table (LUT) for sine and cosine is constructed
at the beginning of the routine.

\subsubsection*{Uses}
\begin{verbatim}
None
\end{verbatim}

\subsubsection*{Notes}

\vfill{\footnotesize\input{LALDemodCV}}

</lalLaTeX> */

/* loop protection */
#ifndef LALDEMODFAST_C
#define LALDEMODFAST_C
#endif

#include <lal/LALStatusMacros.h>
#include <lal/LALDemod.h>
NRCSID( LALDEMODFASTC, "$Id$" );

/* <lalVerbatim file="LALDemodCP"> */
void LALDemodFAST(LALStatus *status, LALFstat *Fstat, FFT **input, DemodPar *params)
/* </lalVerbatim> */
{

  INT4 alpha,i;                 /* loop indices */
  REAL8	*xSum=NULL, *ySum=NULL;	/* temp variables for computation of fs*as and fs*bs */
  INT4 s;		        /* local variable for spinDwn calcs. */
  REAL8	deltaF;	                /* width of SFT band */
  INT4	k;	                /* defining the sum over which is calculated */
  REAL8 *skyConst;	        /* vector of sky constants data */
  REAL8 *spinDwn;	        /* vector of spinDwn parameters (maybe a structure? */
  INT4	spOrder;	        /* maximum spinDwn order */
  INT4	sftIndex;	        /* more temp variables */
  REAL8 realQ, imagQ;
  INT4 *tempInt1;
  REAL8 FaSq;
  REAL8 FbSq;
  REAL8 FaFb;
  COMPLEX16 Fa, Fb;
  REAL8 *sinVal,*cosVal;        /*LUT values computed by the routine do_trig_lut*/
  INT4  res;                    /*resolution of the argument of the trig functions: 2pi/res.*/
  INT4  reso2;                   /*resolution of the argument of the trig functions divided by 2*/
  INT4  myindex;                /*LUT index*/
  REAL8 Y;                      /*Phase in alpha loop*/

  REAL8 f;
  COMPLEX8 Xalpha_k;
  REAL8 norm, df;

  REAL8 A=params->amcoe->A,B=params->amcoe->B,C=params->amcoe->C,D=params->amcoe->D;
  INT4 M=params->SFTno;
  INT4 ifmin;

  REAL8 N=4.0/(D*M);
  REAL8 TwoC=2.0*C;

  INITSTATUS( status, "LALDemodFAST", LALDEMODFASTC );

  /* catch some obvious programming errors */
  ASSERT ( (Fstat != NULL)&&(Fstat->F != NULL), status, LALDEMODH_ENULL, LALDEMODH_MSGENULL );
  if (params->returnFaFb)
    {
      ASSERT ( (Fstat->Fa != NULL)&&(Fstat->Fb != NULL), status, LALDEMODH_ENULL, LALDEMODH_MSGENULL );
    }

  /* variable redefinitions for code readability */
  spOrder=params->spinDwnOrder;
  spinDwn=params->spinDwn;
  skyConst=params->skyConst;
  deltaF=(*input)->fft->deltaF;

  /* this loop computes the values of the phase model */
  xSum=(REAL8 *)LALMalloc(params->SFTno*sizeof(REAL8));
  ySum=(REAL8 *)LALMalloc(params->SFTno*sizeof(REAL8));
  tempInt1=(INT4 *)LALMalloc(params->SFTno*sizeof(INT4));
  for(alpha=0;alpha<params->SFTno;alpha++){
    tempInt1[alpha]=2*alpha*(spOrder+1)+1;
    xSum[alpha]=0.0;
    ySum[alpha]=0.0;
    for(s=0; s<spOrder;s++) {
      xSum[alpha] += spinDwn[s] * skyConst[tempInt1[alpha]+2+2*s];
      ySum[alpha] += spinDwn[s] * skyConst[tempInt1[alpha]+1+2*s];
    }
  }

  /* res=10*(params->mCohSFT); */
  /* This size LUT gives errors ~ 10^-7 with a three-term Taylor series */
   res=128;
   reso2=res/2;
   sinVal=(REAL8 *)LALMalloc((res+1)*sizeof(REAL8));
   cosVal=(REAL8 *)LALMalloc((res+1)*sizeof(REAL8));
   for (k=0; k<=res; k++){
     sinVal[k]=sin((LAL_TWOPI*(k-reso2))/reso2);
     cosVal[k]=cos((LAL_TWOPI*(k-reso2))/reso2);
   }

  /* fine frequency resolution */
  df=params->df;

  /* normalisation factor from coarse to fine frequency bins; this is
     necessary to readjust skyconstant calculations which return an
     index like variable assuming Tsft rather than Tcoh */
  norm = deltaF/df;

  /* minimum frequency index of SFT input data */
  ifmin = (*input)->fft->f0/df;

  /* Loop over frequencies to be demodulated */
  for(i=0 ; i< params->imax  ; i++ )
  {
    FFT **inputptr=input;
    REAL8 *xSumptr=xSum;
    REAL8 *ySumptr=ySum;
    REAL4 *aptr=params->amcoe->a->data;
    REAL4 *bptr=params->amcoe->b->data;
    INT4 *tempInt1ptr = tempInt1;

    Fa.re = 0.0;
    Fa.im = 0.0;
    Fb.re = 0.0;
    Fb.im = 0.0;

    f = params->f0 + i*df;

    /* Loop over SFTs that contribute to F-stat for a given frequency */
    for(alpha=0;alpha<params->SFTno;alpha++)
      {
	REAL8 realQXP;
	REAL8 imagQXP;
	REAL4 a = *aptr;
	REAL4 b = *bptr;
	INT4  no = *tempInt1ptr;

	/* compute the frequency bin in over-resolved SFTs */
	sftIndex =(INT4) ( (f*skyConst[ no ]+ *xSumptr )*norm - ifmin );

	Xalpha_k=(*inputptr)->fft->data->data[sftIndex];

	/* Here's the LUT version of the phase computation */
	Y = - f*skyConst[ no - 1 ] - *ySumptr;
	Y -= (INT4)Y;

	myindex = (INT4)(Y*reso2+reso2+0.5);
	realQ = cosVal[myindex];
	imagQ = sinVal[myindex];

	realQXP = Xalpha_k.re*realQ-Xalpha_k.im*imagQ;
	imagQXP = Xalpha_k.re*imagQ+Xalpha_k.im*realQ;

	/* amplitude demodulation */
	Fa.re += a*realQXP;
	Fa.im += a*imagQXP;
	Fb.re += b*realQXP;
	Fb.im += b*imagQXP;

	/* advance pointers that are functions of alpha: a, b,
	   tempInt1, xSum, ySum, input */
	inputptr++;
	xSumptr++;
	ySumptr++;
	aptr++;
	bptr++;
	tempInt1ptr++;
      }

    FaSq = Fa.re*Fa.re+Fa.im*Fa.im;
    FbSq = Fb.re*Fb.re+Fb.im*Fb.im;
    FaFb = Fa.re*Fb.re+Fa.im*Fb.im;

    Fstat->F[i] = N*(B*FaSq + A*FbSq - TwoC*FaFb);
    if (params->returnFaFb)
      {
	Fstat->Fa[i] = Fa;
	Fstat->Fb[i] = Fb;
      }
/*     fprintf(stdout, "%e %e\n", f, Fstat->F[i]); */

  }
  /* Clean up */
  LALFree(tempInt1);
  LALFree(xSum);
  LALFree(ySum);

  LALFree(sinVal);
  LALFree(cosVal);

  RETURN( status );

} /* LALDemod() */

/* 	if (sftIndex < 0 || sftIndex > input[alpha]->fft->data->length-1) */
/* 	  { */
/* 	    fprintf(stdout,"Problem: %d %d %d %e %e\n", i, alpha, sftIndex, Xalpha_k.re, Xalpha_k.im); */
/* 	  } */
/*
 *  Copyright (C) 2007 Jolien Creighton, B.S. Sathyaprakash, Thomas Cokelaer
 *  Copyright (C) 2012 Leo Singer, Evan Ochsner, Les Wade, Alex Nitz
 *  Assembled from code found in:
 *    - LALInspiralStationaryPhaseApproximation2.c
 *    - LALInspiralChooseModel.c
 *    - LALInspiralSetup.c
 *    - LALSimInspiralTaylorF2ReducedSpin.c
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

#include <stdlib.h>
#include <math.h>
#include <lal/Date.h>
#include <lal/FrequencySeries.h>
#include <lal/LALConstants.h>
#include <lal/Sequence.h>
#include <lal/LALDatatypes.h>
#include <lal/LALSimInspiral.h>
#include <lal/Units.h>
#include <lal/XLALError.h>
#include "LALSimInspiralPNCoefficients.c"

#ifndef _OPENMP
#define omp ignore
#endif

/**
 * @addtogroup LALSimInspiralTaylorXX_c
 * @{
 *
 * @name Routines for TaylorF2-NLTides Waveforms
 *
 * @{
 */

/** \brief Returns structure containing TaylorF2 phasing coefficients for given
 *  physical parameters.
 */
int XLALSimInspiralTaylorF2AlignedPhasingNLTides(
        PNPhasingSeries **pn,   /**< phasing coefficients (output) */
        const REAL8 m1,         /**< mass of body 1 */
        const REAL8 m2,		/**< mass of body 2 */
        const REAL8 chi1,	/**< aligned spin parameter of body 1 */
        const REAL8 chi2,	/**< aligned spin parameter of body 2 */
        LALDict *p              /**< LAL dictionary containing accessory parameters */
	)
{
    PNPhasingSeries *pfa;

    if (!pn) XLAL_ERROR(XLAL_EFAULT);
    if (*pn) XLAL_ERROR(XLAL_EFAULT);


    pfa = (PNPhasingSeries *) LALMalloc(sizeof(PNPhasingSeries));

    XLALSimInspiralPNPhasing_F2(pfa, m1, m2, chi1, chi2, chi1*chi1, chi2*chi2, chi1*chi2, p);

    *pn = pfa;

    return XLAL_SUCCESS;
}

/*
The function which computes the phase shift as a function of frequency
*/
int XLALSimInspiralTaylorF2NLPhase(
	REAL8Sequence *dphi, /**<the arrary for the NL phase errors, should be the same length as freqs */
        const REAL8Sequence *freqs,     /**< frequency array [Hz], should be the same length as dphi */
        const REAL8 Anl1, /**< the amplitude of the phase shift from m1 */
        const REAL8 n1,     /**< the spectral index of the phase shift from m1 */
        const REAL8 fo1,    /**< the 'turn-on' frequency for the phase shift in m1 [Hz] */
        const REAL8 m1_SI,    /**< the primary mass [kg] */
        const REAL8 Anl2, /**< the amplitude of the phase shift from m2 */
        const REAL8 n2,     /**< the spectral index of the phase shift from m2 */
        const REAL8 fo2,    /**< the 'turn-on' frequency for the phase shift in m2 [Hz]*/
        const REAL8 m2_SI    /**< the secondary mass [kg] */
	)
{
    /*
    We compute this from a basic post-newtonian expansion that includes dissipation from the tides as an extra energy sink
    In particular, we assume 0th order gravitational radiation loss and orbital energy and add terms like
        Edot_1 = 2*N1*Y1*Esat1
    where
        N is the number of modes participating
        Y is the growth rate of the instability
        Esat is the energy at which unstable modes saturate
    Thus
        Edot_1 = 2*pi^2 * m1*m2/(m1+m2) * G^(2/3) * m1*(2/3) * fref^(5/3) * Anl1 * (f/fref)^(2+n1) * Theta(f-fo1)
    and similarly for Edot_2
    Assuming the phase shift introduced is small (Anl <~ 1e-5 for 1.4-1.4 BNS), we expand the resulting integral for the phase as a power series and truncate it after the first term.
    This yields the resulting phase contribution, which is strictly negative.
    */

    // set up constants outside of loop
    REAL8 fref = 100 ; // Hz

    REAL8 Mtot = m1_SI + m2_SI ;
    REAL8 Mchirp = pow((m1_SI*m2_SI), 0.6) / pow(Mtot, 0.2) ;

    REAL8 a1 = n1 - 3. ;
    REAL8 a2 = n2 - 3. ;

    REAL8 C2 = (50. * pow(2., 2./3.) / 3072.) * pow(pow(LAL_C_SI, 3.) / (LAL_G_SI * Mchirp * fref * LAL_PI), 10./3.) ;
    // there is a possible sign error here, which may depend on conventions in LALInference
    REAL8 C1, b1, b2 ;
    if (n1==3){
        C1 = C2 * pow(m1_SI/Mtot, 2./3.) * Anl1 ;
        b1 = log(fo1) ;
    } else { //n1!=3
        C1 = C2 * pow(m1_SI/Mtot, 2./3.) * Anl1 * pow(fref, -a1) / a1 ;
        b1 = pow(fo1, a1) ;
    }
    if (n2==3) {
        C2 = C2 * pow(m2_SI/Mtot, 2./3.) * Anl2 ;
        b2 = log(fo2) ;
    } else { //n2!=3
        C2 = C2 * pow(m2_SI/Mtot, 2./3.) * Anl2 * pow(fref, -a2) / a2 ;
        b2 = pow(fo2, a2) ;
    }

    // iterate through freqs and fill in
        UINT4 i = 0;
    REAL8 f ;
    if (fo1 < fo2 ) {
        while ((i<freqs->length) && (freqs->data[i] < fo1)) {
            dphi->data[i] = 0.0 ;
            i++ ;
        }
        if (n1==3) {
            while ((i<freqs->length) && (freqs->data[i] < fo2)) {
                dphi->data[i] = C1 * ( log(freqs->data[i]) - b1 ) ;
                i++ ;
            }
            if (n2==3) {
                while (i<freqs->length) {
                    f = log( freqs->data[i] ) ;
                    dphi->data[i] = C1 * ( f - b1 ) + C2 * ( f - b2 ) ;
                    i++ ;
                }
            } else { //n2!=3
                while (i<freqs->length) {
                    f = freqs->data[i] ;
                    dphi->data[i] = C1 * ( log( f ) - b1 ) + C2 * ( pow(f, a2) - b2 ) ;
                    i++ ;
                }
            }
        } else { //n1!=3
            while ((i<freqs->length) && (freqs->data[i] < fo2)) {
                dphi->data[i] = C1 * ( pow(freqs->data[i], a1) - b1 ) ;
                i++ ;
            }
            if (n2==3) {
                while (i<freqs->length) {
                    f = freqs->data[i] ;
                    dphi->data[i] = C1 * ( pow(f, a1) - b1 ) + C2 * ( log( f ) - b2 ) ;
                    i++ ;
                }
            } else { //n2!=3
                while (i<freqs->length) {
                    f = freqs->data[i] ;
                    dphi->data[i] = C1 * ( pow(f, a1) - b1 ) + C2 * ( pow(f, a2) - b2 ) ;
                    i++ ;
                }
            }
        }
    } else { // fo2 <= fo1
        while ((i<freqs->length) && (freqs->data[i] < fo2)) {
            dphi->data[i] = 0.0 ;
            i++ ;
        }
        if (n2==3) {
            while ((i<freqs->length) && (freqs->data[i] < fo1)) {
                dphi->data[i] = C2 * ( log(freqs->data[i]) - b2 ) ;
                i++ ;
            }
            if (n1==3) {
                while (i<freqs->length) {
                    f = log( freqs->data[i] ) ;
                    dphi->data[i] = C2 * ( f - b2 ) + C1 * ( f - b1 ) ;
                    i++ ;
                }
            } else { //n1!=3
                while (i<freqs->length) {
                    f = freqs->data[i] ;
                    dphi->data[i] = C2 * ( log( f ) - b2 ) + C1 * ( pow(f, a1) - b1 ) ;
                    i++ ;
                }
            }
        } else { //n2!=3
            while ((i<freqs->length) && (freqs->data[i] < fo1)) {
                dphi->data[i] = C2 * ( pow(freqs->data[i], a2) - b2 ) ;
                i++ ;
            }
            if (n1==3) {
                while (i<freqs->length) {
                    f = freqs->data[i] ;
                    dphi->data[i] = C2 * ( pow(f, a2) - b2 ) + C1 * ( log( f ) - b1 ) ;
                    i++ ;
                }
            } else { //n1!=3
                while (i<freqs->length) {
                    f = freqs->data[i] ;
                    dphi->data[i] = C2 * ( pow(f, a2) - b2 ) + C1 * ( pow(f, a1) - b1 ) ;
                    i++ ;
                }
            }
        }
    }
    return XLAL_SUCCESS;
}

int XLALSimInspiralTaylorF2CoreNLTides(
        COMPLEX16FrequencySeries **htilde_out, /**< FD waveform */
	const REAL8Sequence *freqs,            /**< frequency points at which to evaluate the waveform (Hz) */
        const REAL8 phi_ref,                   /**< reference orbital phase (rad) */
        const REAL8 m1_SI,                     /**< mass of companion 1 (kg) */
        const REAL8 m2_SI,                     /**< mass of companion 2 (kg) */
        const REAL8 S1z,                       /**<  z component of the spin of companion 1 */
        const REAL8 S2z,                       /**<  z component of the spin of companion 2  */
        const REAL8 f_ref,                     /**< Reference GW frequency (Hz) - if 0 reference point is coalescence */
	const REAL8 shft,		       /**< time shift to be applied to frequency-domain phase (sec)*/
        const REAL8 r,                         /**< distance of source (m) */
        LALDict *p /**< Linked list containing the extra testing GR parameters >**/
        )
{

    if (!htilde_out) XLAL_ERROR(XLAL_EFAULT);
    if (!freqs) XLAL_ERROR(XLAL_EFAULT);
    /* external: SI; internal: solar masses */
    const REAL8 m1 = m1_SI / LAL_MSUN_SI;
    const REAL8 m2 = m2_SI / LAL_MSUN_SI;
    const REAL8 m = m1 + m2;
    const REAL8 m_sec = m * LAL_MTSUN_SI;  /* total mass in seconds */
    const REAL8 eta = m1 * m2 / (m * m);
    const REAL8 piM = LAL_PI * m_sec;
    const REAL8 m1OverM = m1 / m;
    const REAL8 m2OverM = m2 / m;
    REAL8 amp0;
    size_t i;
    COMPLEX16 *data = NULL;
    LIGOTimeGPS tC = {0, 0};
    INT4 iStart = 0;

    COMPLEX16FrequencySeries *htilde = NULL;

    if (*htilde_out) { //case when htilde_out has been allocated in XLALSimInspiralTaylorF2
	    htilde = *htilde_out;
	    iStart = htilde->data->length - freqs->length; //index shift to fill pre-allocated data
	    if(iStart < 0) XLAL_ERROR(XLAL_EFAULT);
    }
    else { //otherwise allocate memory here
	    htilde = XLALCreateCOMPLEX16FrequencySeries("htilde: FD waveform", &tC, freqs->data[0], 0., &lalStrainUnit, freqs->length);
	    if (!htilde) XLAL_ERROR(XLAL_EFUNC);
	    XLALUnitMultiply(&htilde->sampleUnits, &htilde->sampleUnits, &lalSecondUnit);
    }

    /* phasing coefficients */
    PNPhasingSeries pfa;
    XLALSimInspiralPNPhasing_F2(&pfa, m1, m2, S1z, S2z, S1z*S1z, S2z*S2z, S1z*S2z, p);

    REAL8 pfaN = 0.; REAL8 pfa1 = 0.;
    REAL8 pfa2 = 0.; REAL8 pfa3 = 0.; REAL8 pfa4 = 0.;
    REAL8 pfa5 = 0.; REAL8 pfl5 = 0.;
    REAL8 pfa6 = 0.; REAL8 pfl6 = 0.;
    REAL8 pfa7 = 0.;

    INT4 phaseO=XLALSimInspiralWaveformParamsLookupPNPhaseOrder(p);
    switch (phaseO)
    {
        case -1:
        case 7:
            pfa7 = pfa.v[7];
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
            __attribute__ ((fallthrough));
#endif
        case 6:
            pfa6 = pfa.v[6];
            pfl6 = pfa.vlogv[6];
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
            __attribute__ ((fallthrough));
#endif
        case 5:
            pfa5 = pfa.v[5];
            pfl5 = pfa.vlogv[5];
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
            __attribute__ ((fallthrough));
#endif
        case 4:
            pfa4 = pfa.v[4];
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
            __attribute__ ((fallthrough));
#endif
        case 3:
            pfa3 = pfa.v[3];
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
            __attribute__ ((fallthrough));
#endif
        case 2:
            pfa2 = pfa.v[2];
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
            __attribute__ ((fallthrough));
#endif
        case 1:
            pfa1 = pfa.v[1];
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
            __attribute__ ((fallthrough));
#endif
        case 0:
            pfaN = pfa.v[0];
            break;
        default:
            XLAL_ERROR(XLAL_ETYPE, "Invalid phase PN order %d", phaseO);
    }

    /* Validate expansion order arguments.
     * This must be done here instead of in the OpenMP parallel loop
     * because when OpenMP parallelization is turned on, early exits
     * from loops (via return or break statements) are not permitted.
     */

    /* Validate amplitude PN order. */
    INT4 amplitudeO=XLALSimInspiralWaveformParamsLookupPNAmplitudeOrder(p);
    switch (amplitudeO)
    {
        case -1:
        case 7:
        case 6:
        case 5:
        case 4:
        case 3:
        case 2:
        case 0:
            break;
        default:
            XLAL_ERROR(XLAL_ETYPE, "Invalid amplitude PN order %d", amplitudeO);
    }

    /* Generate tidal terms separately.
     * Enums specifying tidal order are in LALSimInspiralWaveformFlags.h
     */
    REAL8 pft10 = 0.;
    REAL8 pft12 = 0.;
    REAL8 lambda1=XLALSimInspiralWaveformParamsLookupTidalLambda1(p);
    REAL8 lambda2=XLALSimInspiralWaveformParamsLookupTidalLambda2(p);
    switch( XLALSimInspiralWaveformParamsLookupPNTidalOrder(p) )
    {
        case LAL_SIM_INSPIRAL_TIDAL_ORDER_ALL:
        case LAL_SIM_INSPIRAL_TIDAL_ORDER_6PN:
	    pft12 = pfaN * (lambda1*XLALSimInspiralTaylorF2Phasing_12PNTidalCoeff(m1OverM) + lambda2*XLALSimInspiralTaylorF2Phasing_12PNTidalCoeff(m2OverM) );
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
            __attribute__ ((fallthrough));
#endif
        case LAL_SIM_INSPIRAL_TIDAL_ORDER_5PN:
            pft10 = pfaN * ( lambda1*XLALSimInspiralTaylorF2Phasing_10PNTidalCoeff(m1OverM) + lambda2*XLALSimInspiralTaylorF2Phasing_10PNTidalCoeff(m2OverM) );
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
            __attribute__ ((fallthrough));
#endif
        case LAL_SIM_INSPIRAL_TIDAL_ORDER_0PN:
            break;
        default:
	    XLAL_ERROR(XLAL_EINVAL, "Invalid tidal PN order %d", XLALSimInspiralWaveformParamsLookupPNTidalOrder(p) );
    }

    /* The flux and energy coefficients below are used to compute SPA amplitude corrections */

    /* flux coefficients */
    const REAL8 FTaN = XLALSimInspiralPNFlux_0PNCoeff(eta);
    const REAL8 FTa2 = XLALSimInspiralPNFlux_2PNCoeff(eta);
    const REAL8 FTa3 = XLALSimInspiralPNFlux_3PNCoeff(eta);
    const REAL8 FTa4 = XLALSimInspiralPNFlux_4PNCoeff(eta);
    const REAL8 FTa5 = XLALSimInspiralPNFlux_5PNCoeff(eta);
    const REAL8 FTl6 = XLALSimInspiralPNFlux_6PNLogCoeff(eta);
    const REAL8 FTa6 = XLALSimInspiralPNFlux_6PNCoeff(eta);
    const REAL8 FTa7 = XLALSimInspiralPNFlux_7PNCoeff(eta);

    /* energy coefficients */
    const REAL8 dETaN = 2. * XLALSimInspiralPNEnergy_0PNCoeff(eta);
    const REAL8 dETa1 = 2. * XLALSimInspiralPNEnergy_2PNCoeff(eta);
    const REAL8 dETa2 = 3. * XLALSimInspiralPNEnergy_4PNCoeff(eta);
    const REAL8 dETa3 = 4. * XLALSimInspiralPNEnergy_6PNCoeff(eta);


    /* Perform some initial checks */
    if (m1_SI <= 0) XLAL_ERROR(XLAL_EDOM);
    if (m2_SI <= 0) XLAL_ERROR(XLAL_EDOM);
    if (f_ref < 0) XLAL_ERROR(XLAL_EDOM);
    if (r <= 0) XLAL_ERROR(XLAL_EDOM);

    /* extrinsic parameters */
    amp0 = -4. * m1 * m2 / r * LAL_MRSUN_SI * LAL_MTSUN_SI * sqrt(LAL_PI/12.L);

    data = htilde->data->data;

    /* Compute the SPA phase at the reference point
     * N.B. f_ref == 0 means we define the reference time/phase at "coalescence"
     * when the frequency approaches infinity. In that case,
     * the integrals Eq. 3.15 of arXiv:0907.0700 vanish when evaluated at
     * f_ref == infinity. If f_ref is finite, we must compute the SPA phase
     * evaluated at f_ref, store it as ref_phasing and subtract it off.
     */
    REAL8 ref_phasing = 0.;
    if( f_ref != 0. ) {
        const REAL8 vref = cbrt(piM*f_ref);
        const REAL8 logvref = log(vref);
        const REAL8 v2ref = vref * vref;
        const REAL8 v3ref = vref * v2ref;
        const REAL8 v4ref = vref * v3ref;
        const REAL8 v5ref = vref * v4ref;
        const REAL8 v6ref = vref * v5ref;
        const REAL8 v7ref = vref * v6ref;
        const REAL8 v8ref = vref * v7ref;
        const REAL8 v9ref = vref * v8ref;
        const REAL8 v10ref = vref * v9ref;
        const REAL8 v12ref = v2ref * v10ref;
        ref_phasing += pfa7 * v7ref;
        ref_phasing += (pfa6 + pfl6 * logvref) * v6ref;
        ref_phasing += (pfa5 + pfl5 * logvref) * v5ref;
        ref_phasing += pfa4 * v4ref;
        ref_phasing += pfa3 * v3ref;
        ref_phasing += pfa2 * v2ref;
        ref_phasing += pfa1 * vref;
        ref_phasing += pfaN;

        /* Tidal terms in reference phasing */
        ref_phasing += pft12 * v12ref;
        ref_phasing += pft10 * v10ref;

        ref_phasing /= v5ref;
    } /* End of if(f_ref != 0) block */

    /******************************************************
     * compute phase change due to non-linear tidal effects
    *******************************************************/

    // array storing non-linear phase shift
    REAL8Sequence *nonlinear_phasing = NULL;
    nonlinear_phasing = XLALCreateREAL8Sequence(freqs->length);

    REAL8 Anl1 = XLALSimInspiralWaveformParamsLookupNLTidesA1(p);
    REAL8 n1 = XLALSimInspiralWaveformParamsLookupNLTidesN1(p);
    REAL8 f1 = XLALSimInspiralWaveformParamsLookupNLTidesF1(p);
    REAL8 Anl2 = XLALSimInspiralWaveformParamsLookupNLTidesA2(p);
    REAL8 n2 = XLALSimInspiralWaveformParamsLookupNLTidesN2(p);
    REAL8 f2 = XLALSimInspiralWaveformParamsLookupNLTidesF2(p);

    XLALSimInspiralTaylorF2NLPhase( nonlinear_phasing, freqs, Anl1, n1, f1, m1_SI, Anl2, n2, f2, m2_SI );

    #pragma omp parallel for
    for (i = 0; i < freqs->length; i++) {
        const REAL8 f = freqs->data[i];
        const REAL8 v = cbrt(piM*f);
        const REAL8 logv = log(v);
        const REAL8 v2 = v * v;
        const REAL8 v3 = v * v2;
        const REAL8 v4 = v * v3;
        const REAL8 v5 = v * v4;
        const REAL8 v6 = v * v5;
        const REAL8 v7 = v * v6;
        const REAL8 v8 = v * v7;
        const REAL8 v9 = v * v8;
        const REAL8 v10 = v * v9;
        const REAL8 v12 = v2 * v10;
        REAL8 phasing = 0.;
        REAL8 dEnergy = 0.;
        REAL8 flux = 0.;
        REAL8 amp;

        phasing += pfa7 * v7;
        phasing += (pfa6 + pfl6 * logv) * v6;
        phasing += (pfa5 + pfl5 * logv) * v5;
        phasing += pfa4 * v4;
        phasing += pfa3 * v3;
        phasing += pfa2 * v2;
        phasing += pfa1 * v;
        phasing += pfaN;

        /* Tidal terms in phasing */
        phasing += pft12 * v12;
        phasing += pft10 * v10;

    /* WARNING! Amplitude orders beyond 0 have NOT been reviewed!
     * Use at your own risk. The default is to turn them off.
     * These do not currently include spin corrections.
     * Note that these are not higher PN corrections to the amplitude.
     * They are the corrections to the leading-order amplitude arising
     * from the stationary phase approximation. See for instance
     * Eq 6.9 of arXiv:0810.5336
     */
	switch (amplitudeO)
        {
            case 7:
                flux += FTa7 * v7;
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
                __attribute__ ((fallthrough));
#endif
            case 6:
                flux += (FTa6 + FTl6*logv) * v6;
                dEnergy += dETa3 * v6;
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
                __attribute__ ((fallthrough));
#endif
            case 5:
                flux += FTa5 * v5;
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
                __attribute__ ((fallthrough));
#endif
            case 4:
                flux += FTa4 * v4;
                dEnergy += dETa2 * v4;
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
                __attribute__ ((fallthrough));
#endif
            case 3:
                flux += FTa3 * v3;
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
                __attribute__ ((fallthrough));
#endif
            case 2:
                flux += FTa2 * v2;
                dEnergy += dETa1 * v2;
#if __GNUC__ >= 7 && !defined __INTEL_COMPILER
                __attribute__ ((fallthrough));
#endif
            case -1: /* Default to no SPA amplitude corrections */
            case 0:
                flux += 1.;
                dEnergy += 1.;
        }

        phasing /= v5;
        flux *= FTaN * v10;
        dEnergy *= dETaN * v;
        // Note the factor of 2 b/c phi_ref is orbital phase
        phasing += shft * f - 2.*phi_ref - ref_phasing + nonlinear_phasing->data[i];
        amp = amp0 * sqrt(-dEnergy/flux) * v;
        data[i+iStart] = amp * cos(phasing - LAL_PI_4)
                - amp * sin(phasing - LAL_PI_4) * 1.0j;
    }

    // free memory associated with nonlinear_phasing
    XLALDestroyREAL8Sequence( nonlinear_phasing ) ;

    *htilde_out = htilde;
    return XLAL_SUCCESS;
}

/**
 * Computes the stationary phase approximation to the Fourier transform of
 * a chirp waveform. The amplitude is given by expanding \f$1/\sqrt{\dot{F}}\f$.
 * If the PN order is set to -1, then the highest implemented order is used.
 *
 * @note f_ref is the GW frequency at which phi_ref is defined. The most common
 * choice in the literature is to choose the reference point as "coalescence",
 * when the frequency becomes infinite. This is the behavior of the code when
 * f_ref==0. If f_ref > 0, phi_ref sets the orbital phase at that GW frequency.
 *
 * See arXiv:0810.5336 and arXiv:astro-ph/0504538 for spin corrections
 * to the phasing.
 * See arXiv:1303.7412 for spin-orbit phasing corrections at 3 and 3.5PN order
 *
 * The spin and tidal order enums are defined in LALSimInspiralWaveformFlags.h
 */
int XLALSimInspiralTaylorF2NLTides(
        COMPLEX16FrequencySeries **htilde_out, /**< FD waveform */
        const REAL8 phi_ref,                   /**< reference orbital phase (rad) */
        const REAL8 deltaF,                    /**< frequency resolution */
        const REAL8 m1_SI,                     /**< mass of companion 1 (kg) */
        const REAL8 m2_SI,                     /**< mass of companion 2 (kg) */
        const REAL8 S1z,                       /**<  z component of the spin of companion 1 */
        const REAL8 S2z,                       /**<  z component of the spin of companion 2  */
        const REAL8 fStart,                    /**< start GW frequency (Hz) */
        const REAL8 fEnd,                      /**< highest GW frequency (Hz) of waveform generation - if 0, end at Schwarzschild ISCO */
        const REAL8 f_ref,                     /**< Reference GW frequency (Hz) - if 0 reference point is coalescence */
        const REAL8 r,                         /**< distance of source (m) */
        LALDict *p /**< Linked list containing the extra testing GR parameters >**/
        )
{
    /* external: SI; internal: solar masses */
    const REAL8 m1 = m1_SI / LAL_MSUN_SI;
    const REAL8 m2 = m2_SI / LAL_MSUN_SI;
    const REAL8 m = m1 + m2;
    const REAL8 m_sec = m * LAL_MTSUN_SI;  /* total mass in seconds */
    // const REAL8 eta = m1 * m2 / (m * m);
    const REAL8 piM = LAL_PI * m_sec;
    const REAL8 vISCO = 1. / sqrt(6.);
    const REAL8 fISCO = vISCO * vISCO * vISCO / piM;
    //const REAL8 m1OverM = m1 / m;
    // const REAL8 m2OverM = m2 / m;
    REAL8 shft, f_max;
    size_t i, n;
    INT4 iStart;
    REAL8Sequence *freqs = NULL;
    LIGOTimeGPS tC = {0, 0};
    int ret;

    COMPLEX16FrequencySeries *htilde = NULL;

    /* Perform some initial checks */
    if (!htilde_out) XLAL_ERROR(XLAL_EFAULT);
    if (*htilde_out) XLAL_ERROR(XLAL_EFAULT);
    if (m1_SI <= 0) XLAL_ERROR(XLAL_EDOM);
    if (m2_SI <= 0) XLAL_ERROR(XLAL_EDOM);
    if (fStart <= 0) XLAL_ERROR(XLAL_EDOM);
    if (f_ref < 0) XLAL_ERROR(XLAL_EDOM);
    if (r <= 0) XLAL_ERROR(XLAL_EDOM);

    /* allocate htilde */
    if ( fEnd == 0. ) // End at ISCO
        f_max = fISCO;
    else // End at user-specified freq.
        f_max = fEnd;
    if (f_max <= fStart) XLAL_ERROR(XLAL_EDOM);

    n = (size_t) (f_max / deltaF + 1);
    XLALGPSAdd(&tC, -1 / deltaF);  /* coalesce at t=0 */
    htilde = XLALCreateCOMPLEX16FrequencySeries("htilde: FD waveform", &tC, 0.0, deltaF, &lalStrainUnit, n);
    if (!htilde) XLAL_ERROR(XLAL_EFUNC);
    memset(htilde->data->data, 0, n * sizeof(COMPLEX16));
    XLALUnitMultiply(&htilde->sampleUnits, &htilde->sampleUnits, &lalSecondUnit);

    /* Fill with non-zero vals from fStart to f_max */
    iStart = (INT4) ceil(fStart / deltaF);

    /* Sequence of frequencies where waveform model is to be evaluated */
    freqs = XLALCreateREAL8Sequence(n - iStart);

    /* extrinsic parameters */
    shft = LAL_TWOPI * (tC.gpsSeconds + 1e-9 * tC.gpsNanoSeconds);

    #pragma omp parallel for
    for (i = iStart; i < n; i++) {
        freqs->data[i-iStart] = i * deltaF;
    }
    ret = XLALSimInspiralTaylorF2CoreNLTides(&htilde, freqs, phi_ref, m1_SI, m2_SI,
                                      S1z, S2z, f_ref, shft, r, p);

    XLALDestroyREAL8Sequence(freqs);

    *htilde_out = htilde;

    return ret;
}

/** @} */
/** @} */

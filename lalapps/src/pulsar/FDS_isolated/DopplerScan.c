/************************************ <lalVerbatim file="DopplerScanCV">
Author: Prix, Reinhard
$Id$
************************************* </lalVerbatim> */

/* some parts of this file are based on code from PtoleMeshTest, written
   by Ben Owen and Ian Jones */

/********************************************************** <lalLaTeX>
\subsection{Module \texttt{DopplerScan}
\label{ss:DopplerScan.c}

Module to generate subsequent sky-positions.

\subsubsection*{Prototypes}
\input{DopplerScanCP}
\idx{NextSkyPosition()}

\subsubsection*{Description}

This module generates subsequent sky-positions corresponding to a 
given search area and resolution and possibly taking account of the
detector sensitivity pattern (.. in the future).

\subsubsection*{Algorithm}

\subsubsection*{Uses}

\subsubsection*{Notes}

\vfill{\footnotesize\input{DopplerScanCV}}

******************************************************* </lalLaTeX> */
#include <math.h>

#include <lal/LALStdlib.h>
#include <lal/DetectorSite.h>
#include <lal/StackMetric.h>
#include <lal/AVFactories.h>
#include <lal/LALError.h>
#include <lal/LALXMGRInterface.h>
#include <lal/StringInput.h>

#include "DopplerScan.h"


NRCSID( DOPPLERSCANC, "$Id$" );

/* TwoDMesh() can have either of two preferred directions of meshing: */
enum {
  ORDER_ALPHA_DELTA,
  ORDER_DELTA_ALPHA
};

static int meshOrder = ORDER_DELTA_ALPHA;

/* internal structure to be passed to getMetric() 
 * containing all the parameters used there: */
typedef struct {
  PtoleMetricIn *metricParams;	/* the actual parameters for the metric-functions */
  INT4 useMetric;		/* which metric to use: NONE, PTOLE, COHERENT, ... */
} getMetricParams_t;

/* the SkyScanner can be in one of the following states */
enum {
  STATE_IDLE,   	/* not initialized yet */
  STATE_READY,		/* initialized and ready */
  STATE_FINISHED,
  STATE_LAST
};

#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)

#define TRUE (1==1)
#define FALSE (1==0)

extern INT4 lalDebugLevel;

/* some empty structs for initializations */
static TwoDMeshParamStruc empty_meshpar;
static PtoleMetricIn empty_metricpar;
static DopplerScanGrid empty_grid;

/* internal prototypes */
void getRange( LALStatus *stat, REAL4 y[2], REAL4 x, void *params );
void getMetric( LALStatus *status, REAL4 g[3], REAL4 skypos[2], void *params );

void printGrid (LALStatus *stat, DopplerScanGrid *grid, SkyRegion *region, TwoDMeshParamStruc *meshpar, INT4 useMetric);
void ConvertTwoDMesh2Grid ( LALStatus *stat, DopplerScanGrid **grid, const TwoDMeshNode *mesh2d, const SkyRegion *region );

BOOLEAN pointInPolygon ( const SkyPosition *point, const SkyRegion *polygon );

void gridFlipOrder ( TwoDMeshNode *grid );

SkyPosition thisPoint;
/*----------------------------------------------------------------------*/
/* <lalVerbatim file="DopplerScanCP"> */
void
InitDopplerScan( LALStatus *stat, DopplerScanState *scan, DopplerScanInit init)
{ /* </lalVerbatim> */
  TwoDMeshNode *mesh2d = NULL;
  DopplerScanGrid *node, head = empty_grid;
  TwoDMeshParamStruc meshpar = empty_meshpar;
  PtoleMetricIn metricpar = empty_metricpar;
  INT4 useMetric;
  getMetricParams_t getMetricParams;

  INITSTATUS( stat, "DopplerScanInit", DOPPLERSCANC );
  ATTATCHSTATUSPTR( stat ); /* prepare for call of LAL-subroutines */

  /* This traps coding errors in the calling routine. */
  ASSERT ( scan != NULL, stat, DOPPLERSCANH_ENONULL, DOPPLERSCANH_MSGENONULL );  

  scan->state = STATE_IDLE;  /* uninitialized */

  /* several functions here need that info, so the easiest is to make this global */
  useMetric = init.metricType;

  /* trap some abnormal input */
  if (init.skyRegion == NULL) {
    ABORT (stat,  DOPPLERSCANH_ENULL ,  DOPPLERSCANH_MSGENULL );
  }

  scan->grid = NULL;  
  scan->gridNode = NULL;

  TRY (ParseSkyRegion (stat->statusPtr, &(scan->skyRegion), init.skyRegion ), stat);

  /* treat special case: only one point given */
  if (scan->skyRegion.numVertices == 1)
    useMetric = LAL_METRIC_NONE;	/* make sure we don't try to use metric here */
  else if (scan->skyRegion.numVertices == 2)	/* this is an anomaly! */
    {
      ABORT (stat, DOPPLERSCANH_E2DSKY, DOPPLERSCANH_MSGE2DSKY);
    }

  /* some general mesh-settings are needed in any case (metric or not) */
  meshpar.getRange = getRange;

  if (meshOrder == ORDER_ALPHA_DELTA)
    {
      meshpar.domain[0] = scan->skyRegion.lowerLeft.longitude;
      meshpar.domain[1] = scan->skyRegion.upperRight.longitude;
    }
  else
    {
      meshpar.domain[0] = scan->skyRegion.lowerLeft.latitude;
      meshpar.domain[1] = scan->skyRegion.upperRight.latitude;
    }

  meshpar.rangeParams = (void*) &(scan->skyRegion); 

  switch (useMetric)      
    {
    case LAL_METRIC_NONE:	/* manual stepping */
      scan->dAlpha = fabs(init.dAlpha);
      scan->dDelta = fabs(init.dDelta);
  
      /* ok now we manually set up the complete grid */
      thisPoint = scan->skyRegion.lowerLeft;	/* start from lower-left corner */

      node = &head;		/* start our grid with an empty head */

      while (1)
	{
	  if (pointInPolygon ( &thisPoint, &(scan->skyRegion) ) )
	    {
	      /* prepare this node */
	      node->next = LALCalloc (1, sizeof(DopplerScanGrid));
	      if (node->next == NULL) {
		ABORT (stat, DOPPLERSCANH_EMEM, DOPPLERSCANH_MSGEMEM);
	      }
	      node = node->next;
	      
	      node->alpha = thisPoint.longitude;
	      node->delta = thisPoint.latitude;
	    } /* if pointInPolygon() */
	  
	  thisPoint.latitude += scan->dDelta;
	  
	  if (thisPoint.latitude > scan->skyRegion.upperRight.latitude)
	    {
	      thisPoint.latitude = scan->skyRegion.lowerLeft.latitude;
	      thisPoint.longitude += scan->dAlpha;
	    } 
	  
	  /* this it the break-condition: are we done yet? */
	  if (thisPoint.longitude >= scan->skyRegion.upperRight.longitude + scan->dAlpha)
	    break;
	  
	} /* while(1) */

      scan->grid = head.next;	/* set result: could be NULL! */

      break;
	  
    case LAL_METRIC_PTOLE:
    case LAL_METRIC_COHERENT:

      /* Prepare call of TwoDMesh(): the mesh-parameters */
      meshpar.mThresh = init.metricMismatch;
      meshpar.nIn = 1e6;  	/* maximum nodes in mesh */ /*  FIXME: hardcoded */
      
      /* helper-function: range and metric */
      meshpar.getMetric = getMetric;
      /* and its parameters: metric-parms & which metric */
      getMetricParams.metricParams = &(metricpar);
      getMetricParams.useMetric = useMetric;
      meshpar.metricParams = (void *) &(getMetricParams);

      /* set up the metric parameters proper (using PtoleMetricIn as container-type) */
      metricpar.position.system = COORDINATESYSTEM_EQUATORIAL;
      /* currently, CreateVector's are broken as they don't allow length=0 */
      /*      TRY( LALSCreateVector( stat->statusPtr, &(scan->MetricPar.spindown), 0 ), stat ); 	*/
      /* FIXME: replace when fixed in LAL */
      metricpar.spindown = LALMalloc ( sizeof(REAL4Vector) );
      metricpar.spindown->length=0;
      metricpar.spindown->data=NULL;

      metricpar.epoch = init.obsBegin;
      metricpar.duration = init.obsDuration;
      metricpar.maxFreq = init.fmax;
      metricpar.site = init.Detector.frDetector;

      scan->grid = NULL;      
      /* finally: create the mesh! (ONLY 2D for now!) */
      TRY( LALCreateTwoDMesh( stat->statusPtr, &mesh2d, &meshpar ), stat);

      /* convert this 2D-mesh into our grid-structure, including clipping to the skyRegion */
      TRY (ConvertTwoDMesh2Grid ( stat->statusPtr, &(scan->grid), mesh2d, &(scan->skyRegion) ), stat);

      /* get rid of 2D-mesh */
      TRY (LALDestroyTwoDMesh ( stat->statusPtr,  &mesh2d, 0), stat);

      break;

    default:
      ABORT ( stat, DOPPLERSCANH_EMETRIC, DOPPLERSCANH_MSGEMETRIC);
      break;

    } /* switch (metric) */

  /* NOTE: we want to make sure we return at least one grid-point: 
   * so check if we got one, and if not, we return the
   * first point of the skyRegion-polygon as a grid-point
   */
  if (scan->grid == NULL)
    {
      scan->grid = LALCalloc (1, sizeof(DopplerScanGrid));
      if (scan->grid == NULL) {
	ABORT (stat, DOPPLERSCANH_EMEM, DOPPLERSCANH_MSGEMEM);
      }
      scan->grid->alpha = scan->skyRegion.vertices[0].longitude;
      scan->grid->delta = scan->skyRegion.vertices[0].latitude;
      
    } /* no points found inside of sky-region */

  /* initialize grid-pointer to first node in list */
  scan->gridNode = scan->grid; 	

  /* count number of nodes in our Dopperscan-grid */
  scan->numGridPoints = 0;
  node = scan->grid;
  while (node)
    {
      scan->numGridPoints ++;
      node = node->next;
    }


  if (lalDebugLevel)
    {
      LALPrintError ("\nFinal Scan-grid has %d nodes\n", scan->numGridPoints);
      TRY( printGrid (stat->statusPtr, scan->grid, &(scan->skyRegion), &meshpar, useMetric ), stat);
    }

  if (metricpar.spindown) {
    /* FIXME: this is currently broken in LAL, as length=0 is not allowed */
    /*    TRY (LALSDestroyVector ( stat->statusPtr, &(scan->MetricPar.spindown) ), stat); */
    LALFree (metricpar.spindown);
    metricpar.spindown = NULL;
  }

  scan->state = STATE_READY;

  /* clean up */
  DETATCHSTATUSPTR (stat);

  RETURN( stat );

} /* InitDopplerScan() */


/*----------------------------------------------------------------------*/
/* <lalVerbatim file="DopplerScanCP"> */
void
FreeDopplerScan (LALStatus *stat, DopplerScanState *scan)
{ /* </lalVerbatim> */
  DopplerScanGrid *node, *next;
  INITSTATUS( stat, "FreeDopplerScan", DOPPLERSCANC);
  ATTATCHSTATUSPTR (stat);

  /* This traps coding errors in the calling routine. */
  ASSERT( scan != NULL, stat, DOPPLERSCANH_ENULL, DOPPLERSCANH_MSGENULL );
  
  if ( scan->state == STATE_IDLE )
    LALWarning (stat, "freeing uninitialized DopplerScan.");
  else if ( scan->state != STATE_FINISHED )
    LALWarning (stat, "freeing unfinished DopplerScan.");

  node=scan->grid;
  while (node)
    {
      next = node->next;
      LALFree (node);
      node = next;
    } /* while node */

  scan->grid = scan->gridNode = NULL;

  if (scan->skyRegion.vertices)
    LALFree (scan->skyRegion.vertices);
  scan->skyRegion.vertices = NULL;
    
  scan->state = STATE_IDLE;

  DETATCHSTATUSPTR (stat);
  RETURN( stat );

} /* FreeDopplerScan() */

/*----------------------------------------------------------------------*/
/* <lalVerbatim file="DopplerScanCP"> */
void
NextDopplerPos( LALStatus *stat, DopplerPosition *pos, DopplerScanState *scan)
{ /* </lalVerbatim> */

  INITSTATUS( stat, "NextDopplerPos", DOPPLERSCANC);

  /* This traps coding errors in the calling routine. */
  ASSERT( pos != NULL, stat, DOPPLERSCANH_ENULL, DOPPLERSCANH_MSGENULL );

  pos->finished = 0;

  switch( scan->state )
    {
    case STATE_IDLE:
      ABORT( stat, DOPPLERSCANH_ENOINIT, DOPPLERSCANH_MSGENOINIT );
      break;

    case STATE_READY:  
      if (scan->gridNode == NULL) 	/* we're done */
	{
	  pos->finished = 1;
	  scan->state = STATE_FINISHED;
	}
      else
	{
	  pos->skypos.longitude = scan->gridNode->alpha;
	  pos->skypos.latitude =  scan->gridNode->delta;

	  pos->spindowns.length = 0; /*  FIXME */

	  scan->gridNode = scan->gridNode->next;
	}
      break;
      
    case STATE_FINISHED:
      pos->finished = 1;  /*  signal to caller that we've finished */
      break;
    }

  RETURN( stat );

} /* NextSkyPos() */


/* **********************************************************************
   The following 2 helper-functions for TwoDMesh() have been adapted 
   from Ben's PtoleMeshTest.

   NOTE: Currently we are very expicitly restricted to 2D searches!! 
   FIXME: generalize to N-dimensional parameter-searches
********************************************************************** */

/* ----------------------------------------------------------------------
 * This is the parameter range function as required by TwoDMesh. 
 *
 * NOTE: for the moment we only provide a trival range as defined by the  
 * rectangular parameter-area [ a1, a2 ] x [ d1, d2 ]
 * 
 *----------------------------------------------------------------------*/
void getRange( LALStatus *stat, REAL4 y[2], REAL4 x, void *params )
{
  SkyRegion *region = (SkyRegion*)params;
  REAL4 nix;

  /* Set up shop. */
  INITSTATUS( stat, "getRange", DOPPLERSCANC );
  /*   ATTATCHSTATUSPTR( stat ); */

  nix = x;	/* avoid compiler warning */
  
  /* for now: we return the fixed y-range, indendent of x */
  if (meshOrder == ORDER_ALPHA_DELTA)
    {
      y[0] = region->lowerLeft.latitude;
      y[1] = region->upperRight.latitude;
    }
  else
    {
      y[0] = region->lowerLeft.longitude;
      y[1] = region->upperRight.longitude;
    }


  /* Clean up and leave. */
  /*   DETATCHSTATUSPTR( stat ); */

  RETURN( stat );
} /* getRange() */


/* ----------------------------------------------------------------------
 * This is a wrapper for the metric function as required by TwoDMesh. 
 *
 *
 * NOTE: this will be called by TwoDMesh(), therefore
 * skypos is in internalOrder, which is not necessarily ORDER_ALPHA_DELTA!!
 * 
 *----------------------------------------------------------------------*/
void getMetric( LALStatus *stat, REAL4 g[3], REAL4 skypos[2], void *params )
{
  INT2 dim;  	/* dimension of (full) parameter space (incl. freq) */
  REAL8Vector   *metric = NULL;  /* for output of metric */
  getMetricParams_t *ourParams = (getMetricParams_t*) params;
  PtoleMetricIn *metricpar = ourParams->metricParams;
  INT4 useMetric = ourParams->useMetric;

  /* Set up shop. */
  INITSTATUS( stat, "getMetric", DOPPLERSCANC );
  ATTATCHSTATUSPTR( stat );

  /* currently we use only f0, alpha, delta: -> 3D metric */
  dim = 3;

  TRY( LALDCreateVector( stat->statusPtr, &metric, dim*(dim+1)/2 ), stat );

  /* Call the metric function. (Ptole or Coherent, which is handled by wrapper) */
  if (meshOrder == ORDER_ALPHA_DELTA)
    {
      metricpar->position.longitude = skypos[0];
      metricpar->position.latitude =  skypos[1];
    }
  else
    {
      metricpar->position.longitude = skypos[1];
      metricpar->position.latitude =  skypos[0];
    }

  /* before we call the metric: make sure the sky-position  is "normalized" */
  TRY ( LALNormalizeSkyPosition (stat->statusPtr, &(metricpar->position), &(metricpar->position)), stat);

  TRY ( LALMetricWrapper( stat->statusPtr, metric, metricpar, useMetric), stat);

  BEGINFAIL( stat )
    TRY( LALDDestroyVector( stat->statusPtr, &metric ), stat );
  ENDFAIL( stat );

  LALProjectMetric( stat->statusPtr, metric, 0 );
  BEGINFAIL( stat )
    TRY( LALDDestroyVector( stat->statusPtr, &metric ), stat );
  ENDFAIL( stat );


  /* the general indexing scheme is g_ab for a>=b: index = b + a*(a+1)/2 */
#define INDEX_AA (1 + 1*(1+1)/2)	/* g_aa */
#define INDEX_DD (2 + 2*(2+1)/2)	/* g_dd */
#define INDEX_AD (1 + 2*(2+1)/2)	/* g_ad */

  /* Translate output. Careful about the coordinate-order here!! */
  if (meshOrder == ORDER_ALPHA_DELTA)
    {
      g[0] = metric->data[INDEX_AA]; /* gxx */
      g[1] = metric->data[INDEX_DD]; /* gyy */
    }
  else
    {
      g[0] = metric->data[INDEX_DD]; /* gxx */
      g[1] = metric->data[INDEX_AA]; /* gyy */
    }

  g[2] = metric->data[INDEX_AD]; /* gxy = g21: 1 + 2*(2+1)/2 = 4; */

 
  /* Clean up and leave. */
  TRY( LALDDestroyVector( stat->statusPtr, &metric ), stat );
  DETATCHSTATUSPTR( stat );
  RETURN( stat );
} /* getMetric() */


/*----------------------------------------------------------------------
 * Debug helper for mesh and metric stuff
 *----------------------------------------------------------------------*/
#define SPOKES 60  /* spokes for ellipse-plots */
#define NUM_SPINDOWN 0       /* Number of spindown parameters */

void 
printGrid (LALStatus *stat, 
	   DopplerScanGrid *grid, 
	   SkyRegion *region, 
	   TwoDMeshParamStruc *meshpar, 
	   INT4 useMetric)
{
  FILE *fp = NULL;
  DopplerScanGrid *node;
  REAL8Vector  *metric = NULL;   /* Parameter-space metric: for plotting ellipses */
  REAL8 gaa, gad, gdd, angle, smaj, smin;
  REAL8 alpha, delta;
  REAL8 mismatch = meshpar->mThresh;
  UINT4 set, i;
  UINT4 dim;
  getMetricParams_t *getMetricParams = (getMetricParams_t*) meshpar->metricParams;
  PtoleMetricIn *metricPar = NULL;
  const CHAR *xmgrHeader = 
    "@version 50103\n"
    "@title \"Sky-grid\"\n"
    "@world xmin -0.1\n"
    "@world xmax 6.4\n"
    "@world ymin -3.2\n"
    "@world ymax 3.2\n"
    "@xaxis label \"alpha\"\n"
    "@yaxis label \"delta\"\n";

  /* Set up shop. */
  INITSTATUS( stat, "printGrid", DOPPLERSCANC );
  ATTATCHSTATUSPTR( stat );

  if (getMetricParams) {
    metricPar = getMetricParams->metricParams;
  }

  /* currently we use only f0, alpha, delta: -> 3D metric */
  dim = 3;

  fp = fopen ("mesh_debug.agr", "w");

  if( !fp ) {
    ABORT ( stat, DOPPLERSCANH_ESYS, DOPPLERSCANH_MSGESYS );
  }
  
  fprintf (fp, xmgrHeader);

  set = 0;

  /* Plot boundary. */
  fprintf( fp, "@target s%d\n@type xy\n", set );

  for( i = 0; i < region->numVertices; i++ )
    {
      fprintf( fp, "%e %e\n", region->vertices[i].longitude, region->vertices[i].latitude );
    }
  fprintf (fp, "%e %e\n", region->vertices[0].longitude, region->vertices[0].latitude ); /* close contour */

  set ++;

  /* Plot mesh points. */
  fprintf( fp, "@s%d symbol 9\n@s%d symbol size 0.33\n", set, set );
  fprintf( fp, "@s%d line type 0\n", set );
  fprintf( fp, "@target s%d\n@type xy\n", set );

  for( node = grid; node; node = node->next )
  {
    fprintf( fp, "%e %e\n", node->alpha, node->delta );
  }


  if (metricPar)
    {
      set++;

      /* plot ellipses (we need metric for that) */
      TRY( LALDCreateVector( stat->statusPtr, &metric, dim*(dim+1)/2 ), stat);
      node = grid;
      while (node)
	{
	  alpha =  node->alpha;
	  delta =  node->delta;

	  /* Get the metric at this skypos */
	  /* only need the update the position, the other
	   * parameter have been set already ! */
	  metricPar->position.longitude = alpha;
	  metricPar->position.latitude  = delta;

	  /* make sure we "normalize" point before calling metric */
	  TRY( LALNormalizeSkyPosition (stat->statusPtr, &(metricPar->position), &metricPar->position), stat);

	  TRY( LALMetricWrapper( stat->statusPtr, metric, metricPar, useMetric ), stat);

	  TRY( LALProjectMetric( stat->statusPtr, metric, 0 ), stat);

	  gaa = metric->data[INDEX_AA];
	  gad = metric->data[INDEX_AD];
	  gdd = metric->data[INDEX_DD];
	  
	  /* Semiminor axis from larger eigenvalue of metric. */
	  smin = gaa+gdd + sqrt( pow(gaa-gdd,2) + pow(2*gad,2) );
	  smin = sqrt(2.0* mismatch/smin);
	  /* Semiminor axis from smaller eigenvalue of metric. */
	  smaj = gaa+gdd - sqrt( pow(gaa-gdd,2) + pow(2*gad,2) );
	  smaj = sqrt(2.0* mismatch /smaj);
	  
	  /* Angle of semimajor axis with "horizontal" (equator). */
	  angle = atan2( gad, mismatch /smaj/smaj-gdd );
	  if (angle <= -LAL_PI_2) angle += LAL_PI;
	  if (angle > LAL_PI_2) angle -= LAL_PI;

	  /*
	  printf ("alpha=%f delta=%f\ngaa=%f gdd=%f gad=%f\n", alpha, delta, gaa, gdd, gad);
	  printf ("smaj = %f, smin = %f angle=%f\n", smaj, smin, angle);
	  */
	  set ++;
	  /* Print set header. */
	  fprintf( fp, "@target G0.S%d\n@type xy\n", set);
	  fprintf( fp, "@s%d color (0,0,0)\n", set );
	  
	  /* Loop around patch ellipse. */
	  for (i=0; i<=SPOKES; i++) {
	    float c, r, b, x, y;
	    
	    c = LAL_TWOPI*i/SPOKES;
	    x = smaj*cos(c);
	    y = smin*sin(c);
	    r = sqrt( x*x + y*y );
	    b = atan2 ( y, x );
	    fprintf( fp, "%e %e\n", alpha + r*cos(angle+b), delta + r*sin(angle+b) );
	  }
	  
	  node = node -> next;
	  
	} /* while node */
      
      TRY( LALDDestroyVector( stat->statusPtr, &metric ), stat );

    } /* if plotEllipses */
      
  fclose(fp);

  DETATCHSTATUSPTR( stat );
  RETURN (stat);

} /* printGrid */

/*----------------------------------------------------------------------
 * this is a "wrapper" to provide a uniform interface to PtoleMetric() 
 * and CoherentMetric().
 *
 * the parameter structure of PtoleMetric() was used, because it's more compact
 *----------------------------------------------------------------------*/
/* <lalVerbatim file="DopplerScanCP"> */
void
LALMetricWrapper (LALStatus *stat, REAL8Vector *metric, PtoleMetricIn *input, LALMetricType type)
{ /* </lalVerbatim> */
  static MetricParamStruc params;
  static PulsarTimesParamStruc spinParams;
  static PulsarTimesParamStruc baryParams;
  static PulsarTimesParamStruc compParams;
  REAL8Vector *lambda = NULL;
  UINT4 i, nSpin;

  INITSTATUS( stat, "LALMetricWrapper", DOPPLERSCANC );
  ATTATCHSTATUSPTR (stat);

  ASSERT ( input, stat, DOPPLERSCANH_ENULL, DOPPLERSCANH_MSGENULL );
  ASSERT ( input->spindown, stat, DOPPLERSCANH_ENULL, DOPPLERSCANH_MSGENULL );

  switch (type)
    {
    case LAL_METRIC_PTOLE: /* nothing to do here, just call the fct */
      TRY ( LALPtoleMetric (stat->statusPtr, metric, input), stat);
      break;

    case LAL_METRIC_COHERENT:       /* this follows mainly Teviet's example in StackMetricTest.c */
      nSpin = input->spindown->length;

      /* Set up constant parameters for barycentre transformation. */
      baryParams.epoch = input->epoch;
      baryParams.t0 = 0;
      baryParams.latitude = input->site.vertexLatitudeRadians;
      baryParams.longitude = input->site.vertexLongitudeRadians;
      TRY( LALGetEarthTimes( stat->statusPtr, &baryParams ), stat );

      /* Set up constant parameters for spindown transformation. */
      spinParams.epoch = input->epoch;
      spinParams.t0 = 0;
      
      /* Set up constant parameters for composed transformation. */
      compParams.epoch = input->epoch;
      compParams.t1 = LALTBaryPtolemaic;
      compParams.t2 = LALTSpin;
      compParams.dt1 = LALDTBaryPtolemaic;
      compParams.dt2 = LALDTSpin;
      compParams.constants1 = &baryParams;
      compParams.constants2 = &spinParams;
      compParams.nArgs = 2;

      /* Set up input structure for CoherentMetric()  */
      if (nSpin)
	{
	  params.dtCanon = LALDTComp;
	  params.constants = &compParams;
	}
      else
	{
	  params.dtCanon = LALDTBaryPtolemaic;
	  params.constants = &baryParams;
	}

      params.start = 0;
      params.deltaT = (REAL8) input->duration;
      params.n = 1; 	/* only 1 stack */
      params.errors = 0;

      /* Set up the parameter list. */
      TRY ( LALDCreateVector( stat->statusPtr, &lambda, nSpin + 2 + 1 ), stat );

      lambda->data[0] = (REAL8) input->maxFreq;
      lambda->data[1] = (REAL8) input->position.longitude;	/* alpha */
      lambda->data[2] = (REAL8) input->position.latitude;	/* delta */

      if ( nSpin ) 
	{
	  for (i=0; i < nSpin; i++)
	    lambda->data[3 + i] = (REAL8) input->spindown->data[i];
	}

      /* _finally_ we can call the metric */
      TRY ( LALStackMetric( stat->statusPtr, metric, lambda, &params ), stat );
      TRY ( LALDDestroyVector( stat->statusPtr, &lambda ), stat );

      break;

    default:
      ABORT (stat, DOPPLERSCANH_EMETRIC,  DOPPLERSCANH_MSGEMETRIC);
      break;
      
    } /* switch type */

  DETATCHSTATUSPTR (stat);
  RETURN (stat);

} /* LALMetricWrapper() */


/*----------------------------------------------------------------------
 * function for checking if a given point lies inside or outside a given
 * polygon, which is specified by a list of points in a SkyPositionVector
 *
 * NOTE: the list of points must not close on itself, the last point
 * is automatically assumed to be connected to the first 
 * 
 * Alorithm: count the number of intersections of rays emanating to the right
 * from the point with the lines of the polygon: even=> outside, odd=> inside
 *
 * NOTE2: we try to get this algorith to count all boundary-points as 'inside'
 *     we do this by counting intersection to the left _AND_ to the right
 *     and consider the point inside if either of those says its inside...
 *     to this end we even allow points to lie outside by up to eps~1e-14
 *
 * Return : TRUE or FALSE
 *----------------------------------------------------------------------*/
BOOLEAN
pointInPolygon ( const SkyPosition *point, const SkyRegion *polygon )
{
  UINT4 i;
  UINT4 N;
  UINT4 insideLeft, insideRight;
  BOOLEAN inside = 0;
  SkyPosition *vertex;
  REAL8 xinter, v1x, v1y, v2x, v2y, px, py;

  if (!point || !polygon || !polygon->vertices || (polygon->numVertices < 3) )
    return 0;

  vertex = polygon->vertices;
  N = polygon->numVertices; 	/* num of vertices = num of edges */

  insideLeft = insideRight = 0;

  px = point->longitude;
  py = point->latitude;

  for (i=0; i < N; i++)
    {
      v1x = vertex[i].longitude;
      v1y = vertex[i].latitude;
      v2x = vertex[(i+1) % N].longitude;
      v2y = vertex[(i+1) % N].latitude;

      /* pre-select candidate edges */
      if ( (py <  MIN(v1y,  v2y)) || (py >  MAX(v1y, v2y) ) || (v1y == v2y) )
	continue;

      /* now calculate the actual intersection point of the horizontal ray with the edge in question*/
      xinter = v1x + (py - v1y) * (v2x - v1x) / (v2y - v1y);

      if (xinter > px)	      /* intersection lies to the right of point (inclusive) */
	insideLeft ++;

      if (xinter < px)       /* intersection lies to the left of point (exclusive!)*/
	insideRight ++;

    } /* for sides of polygon */

  inside = ( ((insideLeft %2) == 1) || (insideRight %2) == 1);
  return inside;
  
} /* pointInPolygon() */


/*----------------------------------------------------------------------
 * parse a string into a SkyRegion structure: the expected string-format is
 *   " (ra1, dec1), (ra2, dec2), (ra3, dec3), ... "
 *----------------------------------------------------------------------*/
/* <lalVerbatim file="DopplerScanCP"> */
void
ParseSkyRegion (LALStatus *stat, SkyRegion *region, const CHAR *input)
{ /* </lalVerbatim> */
  const CHAR *pos;
  UINT4 i;

  INITSTATUS( stat, "ParseSkyRegion", DOPPLERSCANC );

  ASSERT (region != NULL, stat, DOPPLERSCANH_ENULL, DOPPLERSCANH_MSGENULL);
  ASSERT (region->vertices == NULL, stat, DOPPLERSCANH_ENONULL,  DOPPLERSCANH_MSGENONULL);
  ASSERT (input != NULL, stat, DOPPLERSCANH_ENULL, DOPPLERSCANH_MSGENULL);

  region->numVertices = 0;
  region->lowerLeft.longitude = LAL_TWOPI;
  region->lowerLeft.latitude = LAL_PI;
  region->upperRight.longitude = 0;
  region->upperRight.latitude = 0;

  /* count number of entries (by # of opening parantheses) */
  pos = input;
  while ( (pos = strchr (pos, '(')) != NULL )
    {
      region->numVertices ++;
      pos ++;
    }

  if (region->numVertices == 0) {
    LALPrintError ("Failed to parse sky-region: `%s`\n", input);
    ABORT (stat, DOPPLERSCANH_ESKYREGION, DOPPLERSCANH_MSGESKYREGION);
  }
    
  
  /* allocate list of vertices */
  if ( (region->vertices = LALMalloc (region->numVertices * sizeof (SkyPosition))) == NULL) {
    ABORT (stat, DOPPLERSCANH_EMEM, DOPPLERSCANH_MSGEMEM);
  }

  region->lowerLeft.longitude = LAL_TWOPI;
  region->lowerLeft.latitude  = LAL_PI/2.0;

  region->upperRight.longitude = 0;
  region->upperRight.latitude  = -LAL_PI/2;


  /* and parse list of vertices from input-string */
  pos = input;
  for (i = 0; i < region->numVertices; i++)
    {
      if ( sscanf (pos, "(%" LAL_REAL8_FORMAT ", %" LAL_REAL8_FORMAT ")", 
		   &(region->vertices[i].longitude), &(region->vertices[i].latitude) ) != 2) 
	{
	  ABORT (stat, DOPPLERSCANH_ESKYREGION, DOPPLERSCANH_MSGESKYREGION);
	}

      /* keep track of min's and max's to get the bounding square */
      region->lowerLeft.longitude = MIN (region->lowerLeft.longitude, region->vertices[i].longitude);
      region->lowerLeft.latitude  = MIN (region->lowerLeft.latitude, region->vertices[i].latitude);

      region->upperRight.longitude = MAX (region->upperRight.longitude, region->vertices[i].longitude);
      region->upperRight.latitude  =  MAX (region->upperRight.latitude, region->vertices[i].latitude);


      pos = strchr (pos + 1, '(');

    } /* for numVertices */

  RETURN (stat);

} /* ParseSkyRegion() */

/*----------------------------------------------------------------------
 * Translate a TwoDMesh into a DopplerScanGrid using a SkyRegion for clipping
 * 
 * NOTE: the returned grid will be NULL if there are no points inside the sky-region
 *----------------------------------------------------------------------*/
void 
ConvertTwoDMesh2Grid ( LALStatus *stat, DopplerScanGrid **grid, const TwoDMeshNode *mesh2d, const SkyRegion *region )
{
  const TwoDMeshNode *meshpoint;
  DopplerScanGrid head = empty_grid;
  DopplerScanGrid *node;
  SkyPosition point;

  INITSTATUS( stat, "ConvertTwoDMesh2Grid", DOPPLERSCANC );

  ASSERT ( *grid == NULL, stat, DOPPLERSCANH_ENONULL, DOPPLERSCANH_MSGENONULL );
  ASSERT (region, stat, DOPPLERSCANH_ENULL, DOPPLERSCANH_MSGENULL);
  ASSERT (mesh2d, stat, DOPPLERSCANH_ENULL, DOPPLERSCANH_MSGENULL);

  meshpoint = mesh2d;	/* this is the 2-d mesh from LALTwoDMesh() */

  node = &head;		/* this is our Doppler-grid (empty for now) */
  
  while (meshpoint)
    {
      if (meshOrder == ORDER_ALPHA_DELTA)
	{
	  point.longitude = meshpoint->x;
	  point.latitude = meshpoint->y;
	}
      else
	{
	  point.longitude = meshpoint->y;
	  point.latitude = meshpoint->x;
	}
      
      if (pointInPolygon (&point, region) )
	{
	  /* prepare a new node for this point */
	  if ( (node->next = (DopplerScanGrid*) LALCalloc (1, sizeof(DopplerScanGrid) )) == NULL) {
	    ABORT ( stat, DOPPLERSCANH_EMEM, DOPPLERSCANH_MSGEMEM );
	  }
	  node = node->next;

	  node->alpha = point.longitude;
	  node->delta = point.latitude;

	} /* if point in polygon */

      meshpoint = meshpoint->next;

    } /* while meshpoint */

  
  *grid = head.next;		/* return the final grid (excluding static head) */

  RETURN (stat);

} /* ConvertTwoDMesh2Grid() */

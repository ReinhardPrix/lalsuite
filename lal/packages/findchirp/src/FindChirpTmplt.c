/*----------------------------------------------------------------------- 
 * 
 * File Name: FindChirpTmplt.c
 *
 * Author: Brown, D. A.
 * 
 * Revision: $Id$
 * 
 *-----------------------------------------------------------------------
 */

#include <lal/FindChirpEngine.h>

NRCSID( FINDCHIRPTMPLTC, "$Id$" );

void
LALFindChirpDestroyInspiralBank (
    LALStatus           *status,
    InspiralTemplate   **head
    )
{
  InspiralTemplate    *current;

  INITSTATUS( status, "DestroyInspiralBank", FINDCHIRPTMPLTC );
  ATTATCHSTATUSPTR( status );

  ASSERT( *head, status, FINDCHIRPENGINEH_ENULL, FINDCHIRPENGINEH_MSGENULL );

  /* destroy the inspiral template parameter bank */
  while ( *head )
  {
    current = *head;
    if ( current->fine )
    {
      LALFindChirpDestroyInspiralBank( status->statusPtr, &(current->fine) );
      CHECKSTATUSPTR( status );
    }
    *head = (*head)->next;
    LALFree( current );
  }

  DETATCHSTATUSPTR( status );
  RETURN( status );
}


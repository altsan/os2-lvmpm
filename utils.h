/*****************************************************************************
 ** LVMPM - utils.h                                                         **
 *****************************************************************************
 * This header file defines the various utility routines in utils.c          *
 *****************************************************************************/

void             CentreWindow( HWND hwnd, HWND hwndRelative, ULONG ulFlags );
BOOL             CheckDBCS( void );
void             DrawInsetBorder( HPS hps, RECTL rcl );
void             DrawNice3DBorder( HPS hps, RECTL rcl );
void             DrawOutlineBorder( HPS hps, RECTL rcl );
BOOL             FileExists( PSZ pszFile );
MRESULT EXPENTRY FontPreviewProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
BOOL             GetLanguageFile( PSZ pszFullPath, PSZ pszName );
MRESULT EXPENTRY HorizontalRuleProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY InsetBorderProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
BOOL             MenuItemAddCnd(  HWND  hwndMenu, SHORT sPos, SHORT sID, PSZ pszTitle, SHORT sfStyle );
void             SetContainerFieldTitle( HWND hwndCnr, ULONG ulOffset, PSZ pszNew );


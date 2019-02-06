/*****************************************************************************
 ** LVMPM - utils.h                                                         **
 *****************************************************************************
 * This header file defines the various utility routines in utils.c          *
 *****************************************************************************/

void             CentreWindow( HWND hwnd, HWND hwndRelative, ULONG ulFlags );
void             ResizeDialog( HWND hwnd, LONG cx, LONG cy );
BOOL             CheckDBCS( void );
void             DrawInsetBorder( HPS hps, RECTL rcl );
void             DrawNice3DBorder( HPS hps, RECTL rcl );
void             DrawOutlineBorder( HPS hps, RECTL rcl );
BOOL             FileExists( PSZ pszFile );
MRESULT EXPENTRY FontPreviewProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
BOOL             GetLanguageFile( PSZ pszFullPath, PSZ pszName );
MRESULT EXPENTRY HorizontalRuleProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY InsetBorderProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY OutlineBorderProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
BOOL             MenuItemAddCnd(  HWND  hwndMenu, SHORT sPos, SHORT sID, PSZ pszTitle, SHORT sfStyle );
void             MenuItemEnable( HWND hwndMenu1, HWND hwndMenu2, SHORT sID, BOOL fEnable );
void             SetContainerFieldTitle( HWND hwndCnr, ULONG ulOffset, PSZ pszNew );
BOOL             GetSelectedPartition( HWND hwndDV, PPVCTLDATA ppvd );



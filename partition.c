/*****************************************************************************
 ** LVMPM - partition.c                                                     **
 *****************************************************************************
 * Logic for managing the various partition-related secondary dialogs.       *
 *****************************************************************************/
#include "lvmpm.h"

/* ------------------------------------------------------------------------- *
 * PartitionCreate                                                           *
 *                                                                           *
 * Present the partition creation dialog and respond to it accordingly.      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND       hwnd        : handle of the main program client window       *
 *   PDVMGLOBAL pGlobal     : the main program's global data                 *
 *   ADDRESS    handle      : LVM handle of the selected free space          *
 *   BYTE       fFlags      : input flags for partition creation dialog      *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if a new partition was created, FALSE otherwise.                   *
 * ------------------------------------------------------------------------- */
BOOL PartitionCreate( HWND hwnd, PDVMGLOBAL pGlobal, ADDRESS handle, BYTE fFlags )
{
    DVMCREATEPARMS data = {0};
    USHORT         usBtnID;
    BOOL           bRC = FALSE;
    CARDINAL32     iRC;

    if ( !pGlobal || !pGlobal->disks || !pGlobal->ulDisks )
        return FALSE;

    data.hab         = pGlobal->hab;
    data.hmri        = pGlobal->hmri;
    data.fsProgram   = pGlobal->fsProgram;
    data.fsEngine    = pGlobal->fsEngine;
    data.disks       = pGlobal->disks;
    data.ulDisks     = pGlobal->ulDisks;
    data.ctry        = pGlobal->ctry;
    data.fType       = fFlags;
    data.fBootable   = FALSE;
    data.pszName     = NULL;
    data.cLetter     = '\0';       // not used
    data.ulNumber    = 0;
    data.pPartitions = (PADDRESS) calloc( 1, sizeof(ADDRESS));

    if ( !data.pPartitions) return FALSE;
    data.pPartitions[ 0 ] = handle;
    strcpy( data.szFontDlgs, pGlobal->szFontDlgs );
    strcpy( data.szFontDisks, pGlobal->szFontDisks );

    usBtnID = WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) PartitionCreateWndProc,
                         pGlobal->hmri, IDD_PARTITION_CREATE, &data );
    if ( usBtnID != DID_OK )
        goto cleanup;

    if ( data.ulNumber && data.pPartitions && data.pszName ) {
        LvmCreatePartition( data.pPartitions[ 0 ],
                            MiB_TO_SECS( data.ulNumber ),
                            data.pszName,
                            Automatic,
                            FALSE,
                            (data.fType & PARTITION_TYPE_PRIMARY)? TRUE: FALSE,
                            (data.fType & PARTITION_FLAG_FROMEND)? FALSE: TRUE,
                            &iRC );
        if ( iRC == LVM_ENGINE_NO_ERROR )
            bRC = TRUE;
        else
            PopupEngineError( NULL, iRC, hwnd, pGlobal->hab, pGlobal->hmri );
    }

cleanup:
    if ( data.pszName ) free( data.pszName );
    if ( data.pPartitions) free ( data.pPartitions );

    return bRC;
}


/* ------------------------------------------------------------------------- *
 * PartitionCreateWndProc()                                                  *
 *                                                                           *
 * Dialog procedure for the partition creation dialog.                       *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY PartitionCreateWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PDVMCREATEPARMS       pData;
    Partition_Information_Record pir;
    CHAR       szPartName[ PARTITION_NAME_SIZE+1 ];
    CARDINAL32 rc;
    ULONG      ulMaxSize;
    LONG       lVal;
    BYTE       bConstraint;


    switch( msg ) {

        case WM_INITDLG:
            pData = (PDVMCREATEPARMS) mp2;
            if ( !pData ) {
                WinSendMsg( hwnd, WM_CLOSE, 0, 0 );
                break;
            }

            // Set up the dialog style
            g_pfnRecProc = WinSubclassWindow(
                             WinWindowFromID( hwnd, IDD_DIALOG_INSET ),
                             (pData->fsProgram & FS_APP_PMSTYLE)?
                                InsetBorderProc: OutlineBorderProc );
            WinSubclassWindow( WinWindowFromID( hwnd, IDD_DIALOG_INSET2 ),
                               (pData->fsProgram & FS_APP_PMSTYLE)?
                                 InsetBorderProc: OutlineBorderProc );

            // Set the dialog font
            if ( pData->szFontDlgs[ 0 ] )
                WinSetPresParam( hwnd, PP_FONTNAMESIZE,
                                 strlen( pData->szFontDlgs ) + 1,
                                 (PVOID) pData->szFontDlgs );

            // Set a default name
            PartitionDefaultName( szPartName );
            WinSetDlgItemText( hwnd, IDD_PARTITION_CREATE_NAME_FIELD, szPartName );

            // Set the size spinbutton limits
            pir = LvmGetPartitionInfo( pData->pPartitions[ 0 ], &rc );
            if ( rc != LVM_ENGINE_NO_ERROR ) {
                WinSendMsg( hwnd, WM_CLOSE, 0, 0 );
                break;
            }
            ulMaxSize = SECS_TO_MiB( pir.Usable_Partition_Size );
            WinSendDlgItemMsg( hwnd, IDD_PARTITION_CREATE_SIZE_SPIN, SPBM_SETLIMITS,
                               MPFROMLONG( ulMaxSize ), MPFROMLONG( 1 ));
            WinSendDlgItemMsg( hwnd, IDD_PARTITION_CREATE_SIZE_SPIN,
                               SPBM_SETCURRENTVALUE, MPFROMLONG( ulMaxSize ), MPVOID );

            WinSetDlgItemText( hwnd, IDD_PARTITION_CREATE_SIZE_MB, pData->fsProgram & FS_APP_IECSIZES ? "MiB": "MB");

            // Set the 'primary partition' checkbox state
            bConstraint = PartitionConstraints( pir.Drive_Handle, pData->pPartitions[ 0 ] );
            if ( bConstraint == LVM_CONSTRAINED_PRIMARY ) {
                WinCheckButton( hwnd, IDD_PARTITION_CREATE_PRIMARY, TRUE );
                WinEnableControl( hwnd, IDD_PARTITION_CREATE_PRIMARY, FALSE );
            }
            else if ( bConstraint == LVM_CONSTRAINED_LOGICAL ) {
                WinEnableControl( hwnd, IDD_PARTITION_CREATE_PRIMARY, FALSE );
            }
            else if ( bConstraint == LVM_CONSTRAINED_UNUSABLE ) {
                WinSendMsg( hwnd, WM_CLOSE, 0, 0 );
                break;
            }

            if ( pData->fType & PARTITION_FLAG_VOLUME_FREESPACE ) {
                ResizeDialog( hwnd, 0, 28 );
                WinShowWindow( WinWindowFromID( hwnd, IDD_PARTITION_CREATE_INTRO ), TRUE );
            }
            // Display the dialog
            CentreWindow( hwnd, WinQueryWindow( hwnd, QW_OWNER ), SWP_SHOW | SWP_ACTIVATE );
            return (MRESULT) TRUE;


        case WM_COMMAND:
            switch ( SHORT1FROMMP( mp1 )) {
                case DID_OK:            // Create button
                    // Partition name
                    WinQueryDlgItemText( hwnd, IDD_PARTITION_CREATE_NAME_FIELD,
                                         (LONG) sizeof( szPartName ), szPartName );
                    pData->pszName = strdup( szPartName );

                    // Size
                    WinSendDlgItemMsg( hwnd, IDD_PARTITION_CREATE_SIZE_SPIN, SPBM_QUERYVALUE,
                                       MPFROMP( &lVal ), MPFROM2SHORT( 0, SPBQ_ALWAYSUPDATE ));
                    pData->ulNumber = (ULONG) lVal;         // Use this to store the size

                    // Partition type
                    if ( WinQueryButtonCheckstate( hwnd, IDD_PARTITION_CREATE_PRIMARY ))
                        pData->fType = PARTITION_TYPE_PRIMARY;
                    else
                        pData->fType = PARTITION_TYPE_LOGICAL;

                    // Create from end?
                    if ( WinQueryButtonCheckstate( hwnd, IDD_PARTITION_CREATE_FROM_END ))
                        pData->fType |= PARTITION_FLAG_FROMEND;

                    break;

                case DID_CANCEL:
                    break;
            }
            break;


        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * PartitionDefaultName()                                                    *
 *                                                                           *
 * This is a utility function which generates a 'default' partition name.    *
 * The name generated is '[ A# ]' where '#' is a number such that the name   *
 * is unique.                                                                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ pszName: String buffer to receive the partition name generated.     *
 *                Should be at least PARTITION_NAME_SIZE+1 bytes.            *
 *                                                                           *
 * RETURNS: PSZ                                                              *
 *   A pointer to pszName.                                                   *
 * ------------------------------------------------------------------------- */
PSZ PartitionDefaultName( PSZ pszName )
{
    Drive_Control_Array disks;
    CARDINAL32          rc,
                        ulNumber;

    if ( !pszName ) return NULL;

    disks = LvmGetDisks( &rc );
    if ( rc != LVM_ENGINE_NO_ERROR ) {
        sprintf( pszName, "Partition");
        return pszName;
    }
    ulNumber = 1;
    sprintf( pszName, "[ A%u ]", ulNumber );
    while ( PartitionNameExists( pszName, disks )) {
        ulNumber++;
        sprintf( pszName, "[ A%u ]", ulNumber );
    }
    LvmFreeMem( disks.Drive_Control_Data );

    return pszName;
}


/* ------------------------------------------------------------------------- *
 * PartitionNameExists()                                                     *
 *                                                                           *
 * Detects if a proposed partition name is the same as the name of any       *
 * already-existing partition.                                               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ                 pszName: the partition name to be verified          *
 *   Drive_Control_Array disks  : array of all existing disk drives          *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the volume name already exists; FALSE otherwise.                *
 * ------------------------------------------------------------------------- */
BOOL PartitionNameExists( PSZ pszName, Drive_Control_Array disks )
{
    Partition_Information_Array partitions;
    CARDINAL32 rc,
               i;
    BOOL       fExists = FALSE;

    for ( i = 0; ( i < disks.Count) && !fExists; i++ ) {
        partitions = LvmGetPartitions( disks.Drive_Control_Data[ i ].Drive_Handle, &rc );
        if ( rc == LVM_ENGINE_NO_ERROR ) {
            if ( ! strncmp( partitions.Partition_Array[ i ].Partition_Name,
                            pszName, PARTITION_NAME_SIZE ))
                fExists = TRUE;
        }
        LvmFreeMem( partitions.Partition_Array );
    }

    return fExists;
}


/* ------------------------------------------------------------------------- *
 * PartitionConstraints()                                                    *
 *                                                                           *
 * Determines whether a partition created in the given block of free space   *
 * is required to be either logical or primary.  This constraint depends on  *
 * the current layout of the disk and the partitions on either side.         *
 *                                                                           *
 * If there are 4 primaries already, then the space is unusable.             *
 *                                                                           *
 * If there are 3 primaries and at least 1 logical then:                     *
 *   - The new partition must be logical... BUT:                             *
 *   - if the selected free space is not adjacent to the existing logical    *
 *     partition(s), then the space is unusable.                             *
 *                                                                           *
 * Otherwise:                                                                *
 *   - If the selected free space is between two logicals, then the new      *
 *     partition must be logical.                                            *
 *   - If we are not adjacent to a logical, but there is at least 1 logical  *
 *     already existing, then the partition must be primary.                 *
 *   - If none of the applies, then there is no constraint.                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *     ADDRESS hDisk: LVM handle of the disk drive being queried.            *
 *     ADDRESS hPart: LVM handle of the freespace 'partition' to check.      *
 *                                                                           *
 * RETURNS: BYTE                                                             *
 * ------------------------------------------------------------------------- */
BYTE PartitionConstraints( ADDRESS hDisk, ADDRESS hPart )
{
    Partition_Information_Array partitions;
    CARDINAL32 rc;                      // LVM error code
    BOOL      bLogAdjacent = FALSE,     // Is extent adjacent to an existing logical?
              bLogBetween  = FALSE,     // Is extent between two logicals?
              bUnusable    = FALSE;     // Is extent unusable?
    ULONG     ulPCount = 0,             // Number of primaries on disk
              ulLCount = 0,             // Number of logicals on disk
              ulPart   = 0,             // Index of selected partition
              ulLast,                   // Index of last partition on disk
              i;

    partitions = LvmGetPartitions( hDisk, &rc );
    if ( rc != LVM_ENGINE_NO_ERROR )
        return ( LVM_CONSTRAINED_NONE );

    // Count the number of primary vs logical partitions
    for ( i = 0; i < partitions.Count; i++ ) {
        if ( partitions.Partition_Array[ i ].Partition_Handle == hPart )
            ulPart = i;
        if ( partitions.Partition_Array[ i ].Primary_Partition )
            ulPCount++;
        else
            ulLCount++;
    }
    ulLast = partitions.Count - 1;

    if ( ulPCount >= 4 )                            // Partition table is full
        bUnusable = TRUE;
    else if ( ulPart == 0 ) {                       // Edge case: first partition
        if ( !partitions.Partition_Array[ ulPart+1 ].Primary_Partition )
            bLogAdjacent = TRUE;
    }
    else if ( ulPart == ulLast ) {                  // Edge case: last partition
        if ( !partitions.Partition_Array[ ulPart-1 ].Primary_Partition )
            bLogAdjacent = TRUE;
    }
    else {
        if (( !partitions.Partition_Array[ ulPart+1 ].Primary_Partition ) &&
            ( !partitions.Partition_Array[ ulPart-1 ].Primary_Partition ))
        {
            bLogBetween  = TRUE;
            bLogAdjacent = TRUE;
        }
        else if (( !partitions.Partition_Array[ ulPart+1 ].Primary_Partition ) ||
                 ( !partitions.Partition_Array[ ulPart-1 ].Primary_Partition ))
        {
            bLogAdjacent = TRUE;
        }
    }
    LvmFreeMem( partitions.Partition_Array );

    // Return the correct constraint condition
    if ( bUnusable )
        return ( LVM_CONSTRAINED_UNUSABLE );
    else if ( partitions.Count < 2 )
        // No other partitions, so no constraints
        return ( LVM_CONSTRAINED_NONE );
    else if (( ulPCount == 3 ) && ( ulLCount > 0 )) {
        if ( !bLogAdjacent )
            return ( LVM_CONSTRAINED_UNUSABLE );
        else
            return ( LVM_CONSTRAINED_LOGICAL );
    }
    else if ( bLogBetween )
        return ( LVM_CONSTRAINED_LOGICAL );
    else if (( ulLCount > 0 ) && ( !bLogAdjacent ))
        return ( LVM_CONSTRAINED_PRIMARY );
    else
        return ( LVM_CONSTRAINED_NONE );
}


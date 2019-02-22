/*****************************************************************************
 * logging.c                                                                 *
 *                                                                           *
 * Copyright (C) 2011-2019 Alexander Taylor.                                 *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify it *
 *   under the terms of the GNU General Public License as published by the   *
 *   Free Software Foundation; either version 2 of the License, or (at your  *
 *   option) any later version.                                              *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful, but     *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   General Public License for more details.                                *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License along *
 *   with this program; if not, write to the Free Software Foundation, Inc., *
 *   59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                 *
 *****************************************************************************/
#include "lvmpm.h"
#include <time.h>

/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
FILE * LogFileInit( BOOL fAppend )
{
    FILE  *pLog;                     // log file handle
    PSZ    pszLogPath,               // path to store log file
           pszLogFile;               // filespec of log file
    time_t ltime;                    // current time

    if ( DosScanEnv( LOG_PATH, &pszLogPath ) == NO_ERROR ) {
        pszLogFile = (PSZ) malloc( strlen( pszLogPath ) +
                                   strlen( LOG_FILE ) + 2 );
        sprintf( pszLogFile, "%s\\%s", pszLogPath, LOG_FILE );
        pLog = fopen( pszLogFile, fAppend? "a": "w");
        free( pszLogFile );
    }
    else pLog = fopen( LOG_FILE, fAppend? "a": "w");

    if ( pLog ) {
        time( &ltime );
        fprintf( pLog, "*******************************************************************************\n");
        fprintf( pLog, "LVMPM started: %s\n", ctime( &ltime ));
    }
    return ( pLog );
}


/* ------------------------------------------------------------------------- *
 * Log_DiskInfo()                                                            *
 *                                                                           *
 * Writes a summary of the disk layout to the log file (if logging is        *
 * enabled).                                                                 *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal : Structure containing global program data           *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void Log_DiskInfo( PDVMGLOBAL pGlobal )
{
    Partition_Information_Array partitions;
    ULONG      i, j;
    CARDINAL32 iErr;

    if ( !pGlobal->pLog ) return;

    fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
    fprintf( pGlobal->pLog, "LVM Summary: Physical\n" );
    fprintf( pGlobal->pLog, "%u disk drives reported.\n", pGlobal->ulDisks );

    for ( i = 0; i < pGlobal->ulDisks; i++ ) {
        fprintf( pGlobal->pLog, "...............................................................................\n");
        fprintf( pGlobal->pLog, "D%02u: \"%s\"\n",
                 pGlobal->disks[ i ].iNumber,
                 pGlobal->disks[ i ].szName );
        fprintf( pGlobal->pLog, "     Hnd: 0x%08X    SlNo: %08X   Unus: %u   Corr: %u    PRM: %u   BFlp: %u\n",
                 pGlobal->disks[ i ].handle,
                 pGlobal->disks[ i ].iSerial,
                 pGlobal->disks[ i ].fUnusable,
                 pGlobal->disks[ i ].fCorrupt,
                 pGlobal->disks[ i ].fPRM,
                 pGlobal->disks[ i ].fBigFloppy );
        fprintf( pGlobal->pLog, "     Tot: %-11u   Free: %-11u          C: %-5u   H: %-3u   S: %-u\n",
                 MiB_TO_SECS( pGlobal->disks[ i ].iSize ),
                 pGlobal->disks[ i ].iTotalFree,
                 pGlobal->disks[ i ].iCylinders,
                 pGlobal->disks[ i ].iHeads,
                 pGlobal->disks[ i ].iSectors );

        partitions = LvmGetPartitions( pGlobal->disks[ i ].handle, &iErr );
        if ( iErr != LVM_ENGINE_NO_ERROR ) {
            fprintf( pGlobal->pLog, "\n     Unable to query partitions (error code %u)!\n\n", iErr );
            continue;
        }
        for ( j = 0; j < partitions.Count; j++ ) {
            Log_Partition( pGlobal, j+1, partitions.Partition_Array[ j ] );
        }
        LvmFreeMem( partitions.Partition_Array );
//        fprintf( pGlobal->pLog, "\n");
    }
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void Log_Partition( PDVMGLOBAL pGlobal, ULONG ulNum, Partition_Information_Record pir )
{
    fprintf( pGlobal->pLog, "     . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n");
    fprintf( pGlobal->pLog, "     P%02u: \"%s\"", ulNum, pir.Partition_Name );
    if ( pir.Volume_Handle )
        fprintf( pGlobal->pLog, "  (in volume \"%s\")", pir.Volume_Name );
    fprintf( pGlobal->pLog, "\n");
    fprintf( pGlobal->pLog, "          Hnd: 0x%08X    DrvH: 0x%08X",
             pir.Partition_Handle, pir.Drive_Handle );
    if ( pir.Volume_Handle )
        fprintf( pGlobal->pLog, "     VolH: 0x%08X", pir.Volume_Handle );
    fprintf( pGlobal->pLog, "\n");
    fprintf( pGlobal->pLog, "          Sec: %-10u    Len:  %-10u     Usbl: %-10u (%u MiB)\n",
             pir.Partition_Start, pir.True_Partition_Size, pir.Usable_Partition_Size,
             SECS_TO_MiB( pir.Usable_Partition_Size ));
    fprintf( pGlobal->pLog, "          Sta: %c   Pri: %u    BMgr: %u   Act: %u     OS:   0x%02X",
             pir.Partition_Status ?
                 (pir.Partition_Status == PARTITION_IS_IN_USE? 'U': 'A') :
                 'F',
             pir.Primary_Partition,
             pir.On_Boot_Manager_Menu? 1: 0,
             ( pir.Primary_Partition && ( pir.Active_Flag == ACTIVE_PARTITION ))? 1: 0,
             pir.OS_Flag );
    if ( strlen( pir.File_System_Name ))
        fprintf( pGlobal->pLog, " (%s)", pir.File_System_Name );
    fprintf( pGlobal->pLog, "\n");
}


/* ------------------------------------------------------------------------- *
 * Log_VolumeInfo()                                                          *
 *                                                                           *
 * Writes a summary of the current volumes to the log file (if logging is    *
 * enabled).                                                                 *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PDVMGLOBAL pGlobal : Structure containing global program data           *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void Log_VolumeInfo( PDVMGLOBAL pGlobal )
{
    ULONG   i;

    if ( !pGlobal->pLog ) return;

    fprintf( pGlobal->pLog, "-------------------------------------------------------------------------------\n");
    fprintf( pGlobal->pLog, "LVM Summary: Logical\n" );
    fprintf( pGlobal->pLog, "%u volumes reported.\n", pGlobal->ulVolumes );

    for ( i = 0; i < pGlobal->ulVolumes; i++ ) {
        fprintf( pGlobal->pLog, "\n");
        fprintf( pGlobal->pLog, "%c: \"%s\" (%s)\n",
                 pGlobal->volumes[ i ].cLetter,
                 pGlobal->volumes[ i ].szName,
                 pGlobal->volumes[ i ].szFS );
        fprintf( pGlobal->pLog, "   Preferred: %c   Initial: %c   Type: %s   Status: %u   Device: %u\n",
                 pGlobal->volumes[ i ].cPreference,
                 pGlobal->volumes[ i ].cInitial,
                 pGlobal->volumes[ i ].fCompatibility ? "Compatibility": "LVM",
                 pGlobal->volumes[ i ].fBootable,
                 pGlobal->volumes[ i ].bDevice );
        fprintf( pGlobal->pLog, "   %u MiB\n",
                 pGlobal->volumes[ i ].iSize );
    }

}



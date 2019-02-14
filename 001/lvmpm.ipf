:userdoc.

.nameit symbol=os2 text='OS/2'


.*****************************************************************************
:h1 x=left y=bottom width=100% height=100% res=001.Logical Volume Manager - Introduction
:p.The Logical Volume Manager allows you to create, delete, and modify
volumes and partitions on your computer's disk drives. This program, LVMPM,
is a graphical user interface for the Logical Volume Manager functions.

:p.For an explanation of the terms used in this program, see
:link reftype=hd res=002.Terminology:elink..

:p.For a guide to the graphical user interface, go to the section describing
the :link reftype=hd res=100.Windows in LVMPM:elink..


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% id=terms res=002.Terminology
:p.Below is a summary of some of the terms used by the Logical Volume
Manager and its graphical user interface.

.* NOTE: markup conventions for this glossary:
.* - Bold (hp1) is used for term headings, and also for references to other terms
.*   on this page within term descriptions. Only the first use of a term within
.*   a single entry (including the term heading) should be bold.
.* - Italics are used to emphasize technical concepts which do not have their own
.*   entries in this glossary.
.* - Single quotes are used for minor nomenclature such as alternate terms.

:p.:dl break=all.

:dt.:hp2.Advanced volume:ehp2.
:dd.Advanced volumes, also called &osq.LVM volumes&csq. (the original IBM
term), are not compatible with most other operating systems. Their most
significant feature is the ability to link multiple partitions together into a
single volume, with a single name, drive letter, and file system.

:dt.:hp2.Disk:ehp2.
:dd.While this term has a number of uses in general computing, within LVM it
refers to a :hp1.partitioned direct-access storage device:ehp1.. LVM
distinguishes between two types of disks&colon. :hp2.fixed disks:ehp2. and
:hp2.partitioned removable media:ehp2. (PRM).

:dt.:hp2.Drive letter:ehp2.
:dd.An alphabetical letter used to represent a volume.  A volume requires a drive
letter in order to be usable under &os2..

:dt.:hp2.File system:ehp2.
:dd.A file system defines the data format in which files and folders are stored
on a volume. A volume must be formatted with a file system before it can be used
to store data. Selecting file systems and formatting volumes are performed using
&os2. system tools, and are tasks that lie outside the scope of LVM.

:dt.:hp2.Fixed disk:ehp2.
:dd.A fixed disk is a partitionable disk (generally a hard disk or solid state
drive) which is permanently attached to the computer via a local bus interface.

:dt.:hp2.Free space:ehp2.
:dd.Any area of disk space which is not allocated to a partition.

:dt.:hp2.KB:ehp2. or :hp2.KiB:ehp2.
.br
:hp2.MB:ehp2. or :hp2.MiB:ehp2.
.br
:hp2.GB:ehp2. or :hp2.GiB:ehp2.
.br
:hp2.TB:ehp2. or :hp2.TiB:ehp2.
:dd.Units of storage size. For consistency, both forms of notation mean the
same thing throughout LVM&colon. specifically, standard binary (base-two) units
of storage. Thus&colon. one KB (or one KiB) is 1024 bytes;  one MB (or one MiB)
is 1024 KB, and one GB (or one GiB) is 1024 MB.
[:link reftype=fn refid=iec.Note:elink.]

:dt.:hp2.Large floppy:ehp2.
:dd.A removable storage device which presents itself to &os2. as a single,
partitionless block of storage. Some USB devices present themselves this way.
Large floppies are reported by LVM as volumes on non-LVM devices.
:p.Note that &os2. only supports large floppies of up to 2 GB in size. Such
devices prepared for use under other operating systems may be larger than this,
in which case &os2. cannot use them unless they are modified to use normal
(MBR style) partitioning.

:dt.:hp2.Logical Volume Manager:ehp2.
:dd.The Logical Volume Manager (or LVM) is the disk storage management subsystem
of &os2., for which this program is a user interface.

:dt.:hp2.Partition:ehp2.
:dd.A contiguous region of a disk allocated for storage. At present, all
partitions usable by LVM must be defined according to the standard MBR
partition table scheme.
:p.There are two types of partition, &osq.primary&csq. and &osq.logical&csq..
As far as &os2. is concerned, there is little practical difference between
the two types.  However, if you have other operating systems installed on the
same computer, you may be required to keep them on primary partitions.
:p.A single disk drive may be allocated into a maximum of four primary
partitions, or three primary partitions and any number of consecutive logical
partitions.

:dt.:hp2.Partitioned removable media:ehp2.
:dd.Partitioned removable media, or PRM, is a general term for removable storage
devices which are capable of holding partitions and volumes. Such devices include
USB flash drives or external hard disks, memory cards, and IOMega ZIP(R) drives,
among others.
:p.PRM devices which are defined by &os2. but do not have the actual storage media
attached are reported by LVM as empty disks with no partitions.

:dt.:hp2.Standard volume:ehp2.
:dd.A standard volume is the basic volume type in LVM. It consists of a single
partition which has been designated as a volume by LVM. Standard volumes are
compatible with other operating systems, which simply treat them as ordinary
partitions (according to their own disk management logic).
:p.Standard volumes are sometimes referred to as &osq.compatibility
volumes&csq., which is the IBM terminology for them.

:dt.:hp2.Volume:ehp2.
:dd.A volume is essentially a virtual drive&colon. named storage which may
have a drive letter and a file system.
:p.Generally speaking, volumes are simply partitions which have been designated
as volumes by LVM, and are thus capable of being used as storage under &os2..
However, volumes have several special features which distinguish them from
partitions&colon.
:ul.
:li.Not all partitions are necessarily volumes. For example, partitions that
are used by other operating systems on the same computer might not be.  Such
partitions cannot be used by &os2. unless they are converted (or added) to
a volume.
:li.Multiple partitions can be combined into a single volume. This is only
possible with the Advanced (or &osq.LVM&csq.) type of volume.
:li.Any device with a drive letter assigned to it under &os2. is seen as a
volume, even if it does not correspond to any fixed-disk partition(s).  For
example, optical (CD/DVD/BD) drives, network-attached LAN drives, or other
virtual drives are seen by LVM as :hp1.non-LVM devices:ehp1.. They can be
accessed from the operating system like normal volumes, but LVM cannot
manage or modify them.
:eul.
:p.There are two different types of volume available in LVM&colon.
:hp2.standard volumes:ehp2. and :hp2.advanced volumes:ehp2.. See the
corresponding entries for details.

:edl.

.* ------------------------------------------------------------------------
:fn id=iec.
:p.&osq.KB&csq., &osq.MB&csq., &osq.GB&csq., and &osq.TB&csq. are the
traditional notation; &osq.KiB&csq., &osq.MiB&csq., &osq.GiB&csq., and
&osq.TiB&csq. are a newer notation proposed by IEC. The command-line LVM
utility, and most existing &os2. software, uses the traditional notation,
and hence that is the default in the graphical user interface as well.
:p.If you prefer, you can change to using the IEC notation throughout the GUI
instead, via the :link reftype=hd res=1100.Preferences:elink. dialog.
:efn.


.*****************************************************************************
:h1 x=left y=bottom width=100% height=100% res=100.Windows in LVMPM
:p.The main window consists of top and bottom areas, separated by a moveable
split-bar control. The top area is a :hp1.logical view:ehp1. of system storage,
showing information about volumes. The bottom area represents a :hp1.physical
view:ehp1. of the disks attached to the computer.

:p.:hp7.Logical view (top):ehp7.

:p.This area shows information about the volumes that exist on the system.
By default, all volumes known to LVM are shown &ndash. this includes both
volumes controlled by LVM, and those which correspond to non-LVM devices such
as optical drives or network shares. (You can choose to hide volumes on
non-LVM devices by setting the corresponding option in
:link reftype=hd res=1100.Preferences:elink..)

:p.The right-hand side of this area is a status panel showing details about
the currently-selected volume.

:p.You can select operations on a volume by highlighting the desired volume
and either accessing the :hp2.Volumes:ehp2. menu from the menu-bar, or by
right-clicking on the volume to bring up its context menu. The menu will
show all available volume operations; those which are not supported for the
current volume will be disabled automatically.

:p.Whenever you select a volume in the list, the partition (or partitions)
belonging to that volume will appear cross-hatched in the physical view at
the bottom part of the window.  (Note, however, that this will not actually
change the currently-selected partition or disk in the physical view.)

:p.:hp7.Physical view (bottom):ehp7.

:p.This area displays the system's disk drives and the partitions on them. By
default, this shows all disks, including allocated but empty removable media
drives. (You can configure the GUI to hide unavailable removable media drives
by setting the corresponding option in :link reftype=hd res=1100.Preferences:elink..)

:p.At the very bottom of the window is a status bar that shows information
about the currently-selected partition. (This status bar is only enabled when
the disk list has input focus.)

:p.You can select operations on a partition by right-clicking on it to bring up
the partition context menu. This menu shows all available partition operations;
those which are not supported for the current partition will be disabled
automatically.

.* .br
.* :p.:hp1. ...more to come:ehp1.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=200.Disk Name
:p.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=300.Boot Manager Options
:p.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=400.Volume Creation Dialog
:p.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=500.Partition Selection Dialog
:p.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=600.Volume Letter
:p.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=700.Volume Name
:p.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=800.Create Partition
:p.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=1000.Add Partition to Volume
:p.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=1100.Preferences
:p.The :hp2.Preferences:ehp2. dialog allows you to customize certain aspects
of the Logical Volume Manager's appearance and behaviour.

:dl break=all.
:dt.:hp2.Use IBM terminology for volume type:ehp2.
:dd.This option will cause standard volumes to be identified as
&osq.compatibility&csq. volumes, and advanced volumes as &osq.LVM&csq.
volumes.  This reflects the terminology originally introduced by IBM in OS/2
Warp Server for e-business.  This is purely a cosmetic preference for users who
are more comfortable with the older IBM terms.
:dt.:hp2.Use IEC terminology for binary sizes:ehp2.
:dd.In recent years, new abbreviations for binary byte units (KiB, MiB, GiB
instead of KB, MB, GB, etc.) have started to enter popular use.  The IEC has
issued a standard for the use of these new abbreviations to describe storage
sizes.  This option causes the new abbreviations to be used throughout the
graphical user interface.
:dt.:hp2.Warn when no bootable volumes exist:ehp2.
:dd.If this option is enabled, LVMPM will pop up a warning message on program
exit if no bootable volumes exist.
:dt.:hp2.Hide empty removable media drives:ehp2.
:dd.
:dt.:hp2.Hide volumes not managed by LVM:ehp2.
:dd.
:dt.:hp2.Visual style:ehp2.
:dd.
:dt.:hp2.Enable IBM Boot Manager installation:ehp2.
:dd.
:dt.:hp2.Enable Air-Boot installation:ehp2.
:dd.
:edl.

.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% res=1200.Fonts
:p.The :hp2.Fonts:ehp2. dialog allows you to configure the screen fonts used
in various parts of the user interface.


.*****************************************************************************

.im 001\errors.ipf


.*****************************************************************************
:h1 x=left y=bottom width=100% height=100% res=9900.Notices
:p.:hp2.Logical Volume Manager for Presentation Manager:ehp2.
.br
(C) 2011-2019 Alexander Taylor

:lm margin=4.
:p.This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

:p.This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

:p.You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
:lm margin=1.

:p.See :link reftype=hd refid=license.the following section:elink. for the
full text of the GNU General Public License.


.* ------------------------------------------------------------------------
:h2 x=left y=bottom width=100% height=100% id=license res=9910.License
.im 001\license.ipf

:euserdoc.



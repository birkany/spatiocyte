#!/usr/bin/env python

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#
#       This file is part of E-CELL Model Editor package
#
#               Copyright (C) 1996-2003 Keio University
#
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#
#
# E-CELL is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# E-CELL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with E-CELL -- see the file COPYING.
# If not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#END_HEADER
#
#'Design: Gabor Bereczki <gabor@e-cell.org>',
#'Design and application Framework: Kouichi Takahashi <shafi@e-cell.org>',
#'Programming: Gabor Bereczki' at
# E-CELL Project, Lab. for Bioinformatics, Keio University.
#

from Utils import *
import gtk

import os
import os.path
import gtk.gdk

from ModelEditor import *

class PopupMenu:


    def __init__( self, aModelEditor ):

        self.theModelEditor= aModelEditor


    def open( self,  anEvent):
        """
        in: 
            anEvent that triggered popup of menu
        """

        # create Menu
        aMenu = gtk.Menu()

        # create add, decide whether it can be made sensitive 
        aComponent = self.theModelEditor.getLastUsedComponent()
         
        menuList = aComponent.getMenuItems()
        for menu in menuList:
            if menu[0]==None:
                aMenu.append( menu[1] )
            else:
                aMenu.append( self.__createMenuItem( menu[0], menu[1] ) )
        

        aMenu.show_all()

        aMenu.popup(None, None, None, anEvent.button, anEvent.time)
    
        self.theMenu = aMenu



    def __button_pushed( self, *args ):
        """
        signal handler for menuitems choosen
        """
        aMenuItem = args[0]
        
        aName = aMenuItem.get_data( 'Name' )

        self.theMenu.destroy()

        self.theModelEditor.getLastUsedComponent().applyMenuItem(aName )



    def __createMenuItem( self, aName, isSensitive ):
        """
        in:     str aName of menuitem
            bool isSensitive
        """
        if isSensitive == None:
            return
        aMenuItem = gtk.MenuItem(aName)
        
             
        if isSensitive:
    
            # attach signal handler
            aMenuItem.connect( 'activate', self.__button_pushed )
            aMenuItem.set_data( 'Name', aName )
            
        else:
            
            # set insensitive
            aMenuItem.set_sensitive( gtk.FALSE )

        return aMenuItem


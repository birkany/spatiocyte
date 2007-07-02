#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#
#       This file is part of the E-Cell System
#
#       Copyright (C) 1996-2007 Keio University
#       Copyright (C) 2005-2007 The Molecular Sciences Institute
#
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#
#
# E-Cell System is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# E-Cell System is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public
# License along with E-Cell System -- see the file COPYING.
# If not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
# 
#END_HEADER
#
# written by Masahiro Sugimoto <sugi@bioinformatics.org> at
# E-Cell Project, Lab. for Bioinformatics, Keio University.
#

import gtk 
import os
from OsogoWindow import OsogoWindow
from ConfirmWindow import *

class LoggingPolicy( OsogoWindow ):
    def __init__( self ):
        """
        This is confirm popup window class.

        OK_MODE        : The window has 'OK' button.
        OK_CANCEL_MODE : The window has 'OK' and 'Cancel' button.

        When OK is clicked, return OK_PRESSED
        When Cancel is clicked or close Window, return CANCEL_PRESSED
        """
        OsogoWindow.__init__( self )
        self.___num = CANCEL_PRESSED

    def setLoggingPolicy( self, aPolicy ):
        self.theLoggingPolicy = aPolicy

    def initUI( self ):
        OsogoWindow.initUI( self )
        self.populateDialog( self.theLoggingPolicy )

        # add handlers
        self.addHandlers(
            {
                'on_ok_button_clicked': self.oKButtonClicked,
                'on_cancel_button_clicked': self.cancelButtonClicked,
                "on_space_max_toggled" : self.__buttonChosen,
                "on_space_no_limit_toggled" : self.__buttonChosen,
                "on_end_overwrite_toggled" : self.__buttonChosen,
                "on_end_throw_ex_toggled" : self.__buttonChosen,
                "on_log_by_secs_toggled" : self.__buttonChosen,
                "on_log_by_step_toggled" : self.__buttonChosen
                }
            )

    def openConfirmWindow(self,  aMessage, aTitle, isCancel = 1 ):
        """
        pops up a modal dialog window with aTitle (str) as its title
        and displaying aMessage as its message
        and with an OK and a Cancel button

        returns:
        True if Ok button is pressed
        False if cancel button is pressed
        """
        aConfirmWindow = ConfirmWindow(isCancel, aMessage, aTitle )
        return aConfirmWindow.return_result() == OK_PRESSED

    def populateDialog( self, aLoggingPolicy ):
        if aLoggingPolicy[0]>0:
            self['log_by_step'].set_active( True )
            self['step_entry'].set_text( str(aLoggingPolicy[0] ))
            self['second_entry'].set_sensitive( False )
            self['step_entry'].set_sensitive( True )
        else:
            self['log_by_secs'].set_active( True )
            self['second_entry'].set_text( str(aLoggingPolicy[1] ))
            self['second_entry'].set_sensitive( True )
            self['step_entry'].set_sensitive( False )
        if aLoggingPolicy[2]== 0:
            self['end_throw_ex'].set_active( True )
        else:
            self['end_overwrite'].set_active( True )

        if aLoggingPolicy[3] == 0:
            self['space_no_limit'].set_active ( True )
            self['space_entry'].set_sensitive( False )
        else:
            self['spac_max'].set_active( True )
            self['space_entry'].set_text( str( aLoggingPolicy[3] ) )
            self['space_entry'].set_sensitive( True )
            
    def __depopulateDialog( self ):
        aLoggingPolicy = [1,0,0,0]
        if self['log_by_step'].get_active() == True:
            try:
                num = self['step_entry'].get_text()
                aLoggingPolicy[0] = int(num)
                if aLoggingPolicy[0]<1:
                    a=1/0
                aLoggingPolicy[1] = 0
            except:
                self.openConfirmWindow( "Please enter valid positive integer for minimum step size", "Invalid number format", 0)
                return None
        else:
            try:
                aLoggingPolicy[1] = float(self['second_entry'].get_text())
                if aLoggingPolicy[1]<0:
                    a=1/0
                aLoggingPolicy[0] = 0
            except:
                self.openConfirmWindow( "Please enter valid non-negative number for minimum timeinterval", "Invalid number format", 0)
                return None
        if self['end_overwrite'].get_active() == True :
            aLoggingPolicy[2] = 1
        else:
            aLoggingPolicy[2] = 0
        if self['spac_max'].get_active() == True:
            try:
                aLoggingPolicy[3] = int(self['space_entry'].get_text())
                if aLoggingPolicy[3]<0:
                    a=1/0
            except:
                self.openConfirmWindow( "Please enter valid integer for maximum disk size", "Invalid number format", 0)
                return None
        else:
            aLoggingPolicy[3] = 0
        return aLoggingPolicy

    def __buttonChosen( self, *args ):
        aName = args[0].get_name()
        if aName == "log_by_secs":
            self['second_entry'].set_sensitive( True )
            self['step_entry'].set_sensitive( False )
        elif aName == "log_by_step":
            self['second_entry'].set_sensitive( False )
            self['step_entry'].set_sensitive( True )
        elif aName == "space_no_limit":
            self['space_entry'].set_sensitive( False )
        elif aName == "spac_max":
            self['space_entry'].set_sensitive( True )

    def oKButtonClicked( self, *arg ):
        """
        If OK button clicked or the return pressed, this method is called.
        """

        # sets the return number
        aLogPolicy = self.__depopulateDialog()
        if aLogPolicy == None:
            return
        self.___num = aLogPolicy
        self.destroy()

    def cancelButtonClicked( self, *arg ):
        """
        If Cancel button clicked or the return pressed, this method is called.
        """

        # set the return number
        self.___num = None
        self.destroy()
    
    def return_result( self ):
        """
        Returns result
        """
        return self.___num

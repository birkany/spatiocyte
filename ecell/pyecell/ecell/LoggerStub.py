#!/usr/bin/env python

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#
#        This file is part of E-CELL Session Monitor package
#
#                Copyright (C) 1996-2002 Keio University
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
#'Design: Kouichi Takahashi <shafi@e-cell.org>',
#'Programming: Masahiro Sugimoto <sugi@bioinformatics.org>'
#
# E-CELL Project, Lab. for Bioinformatics, Keio University.
#

#from ecell.ObjectStub import *
from ObjectStub import *

# ---------------------------------------------------------------
# LoggerStub -> ObjectStub
#   - provides an object-oriented appearance to the ecs.Simulator's Logger API
#   - does not check validation of each argument.
# ---------------------------------------------------------------

# FIXME: Logger isn't a PropertyInterface
class LoggerStub( ObjectStub ):


	# ---------------------------------------------------------------
	# Constructor
	#
	# aSimulator : a reference to a Simulator
	# aFullPNString : a FullID of the Entity as a String.
	#
	# return -> None
	# This method can throw exceptions.
	# ---------------------------------------------------------------
	def __init__( self, aSimulator, aFullPNString ):

		ObjectStub.__init__( self, aSimulator )
		
		self.theFullPNString = aFullPNString

	# end of __init__

	def getName( self ):
		return self.theFullPNString


	# ---------------------------------------------------------------
	# getLogger
	#
	# return -> None
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def create( self ):

		self.theSimulator.createLogger( self.theFullPNString )

	# end of createLogger

	# ---------------------------------------------------------------
	# deleteLogger
	#
	# return -> None
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def delete( self ):

		self.theSimulator.deleteLogger( self.theFullPNString )

	# end of createLogger


	# ---------------------------------------------------------------
	# exists
	#
	# return -> exist:TRUE / not exist:FALSE
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def exists( self ):

		# When the FullPN of this instance exists in 
		# the FullPN list, returns TRUE
		if self.theFullPNString in self.theSimulator.getLoggerList():
			return TRUE
		else:
			return FALSE

	# end of exists


	# ---------------------------------------------------------------
	# getData
	#
	# return -> data (matrix of double)
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def getData( self, aStartTime=None, anEndTime=None, anInterval=None ):

		if aStartTime == None:
			aStartTime = self.getStartTime()

		if anEndTime == None:
			anEndTime = self.getEndTime()

		if anInterval == None:

			return self.theSimulator.getLoggerData( self.theFullPNString,
			                                        aStartTime,
			                                        anEndTime )

		else:

			return self.theSimulator.getLoggerData( self.theFullPNString,
			                                        aStartTime,
			                                        anEndTime,
			                                        anInterval )
			                                        

	# end of getData

	# ---------------------------------------------------------------
	# getDataWithStartEnd
	#
	# aStartTime : a start time
	# anEndTime  : an end time
	#
	# return -> data (matrix of double)
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def getDataWithStartEnd( self, aStartTime, anEndTime ):

		return self.theSimulator.getLoggerData( self.theFullPNString, aStartTime, anEndTime )

	# end of getDataWithStartEnd


	# ---------------------------------------------------------------
	# getDataWithStartEndInterval
	#
	# aStartTime : a start time
	# anEndTime  : an end time
	# anInterval : an interval
	#
	# return -> data (matrix of double)
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def getDataWithStartEndInterval( self, aStartTime, anEndTime, anInterval ):

		return self.theSimulator.getLoggerData( self.theFullPNString, aStartTime, anEndTime, anInterval )

	# end of getDataWithStartEndInterval


	# ---------------------------------------------------------------
	# getStartTime
	#
	# return -> the start time
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def getStartTime( self ):

		return self.theSimulator.getLoggerStartTime( self.theFullPNString )

	# end of getStartTime


	# ---------------------------------------------------------------
	# getEndTime
	#
	# return -> the end time
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def getEndTime( self ):

		return self.theSimulator.getLoggerEndTime( self.theFullPNString )

	# end of getEndTime


	# ---------------------------------------------------------------
	# getSize
	#
	# return -> the end time
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def getSize( self ):

		return self.theSimulator.getLoggerSize( self.theFullPNString )

	# end of getSize


	# ---------------------------------------------------------------
	# setMinimumInterval
	#
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def setMinimumInterval( self, anInterval ):
		print "setMinimumInterval will be deprecated. use setLoggerPolicy instead."
		return self.theSimulator.setLoggerMinimumInterval( self.theFullPNString, anInterval )

	# end of setMinimumInterval


	# ---------------------------------------------------------------
	# getMinimumInterval
	#
	# return -> the minimum interval
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def getMinimumInterval( self ):
		print "getMinimumInterval will be deprecated. use getLoggerPolicy instead."
		return self.theSimulator.getLoggerMinimumInterval( self.theFullPNString )

	# end of getMinimumInterval

	# ---------------------------------------------------------------
	# getLoggerPolicy
	#
	# return -> the logger policy
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def getLoggerPolicy( self ):
		return self.theSimulator.getLoggerPolicy( self.theFullPNString )

	#end of getLoggerPolicy

	# ---------------------------------------------------------------
	# setLoggerPolicy
	#
	# return -> the logger policy
	# tuple of 4 numbers 
	# first number : minimum step count
	# second number : minimum time interval
	# third number : policy when disk space or allocataed storage is used up : 0 throw exeption, 1 overwrite old data
	# fourth number : max allocated space by logger in kilobytes.
	# This method can throw exceptions.
	# ---------------------------------------------------------------

	def setLoggerPolicy( self, aLoggingPolicy ):
		return self.theSimulator.setLoggerPolicy( self.theFullPNString, aLoggingPolicy)

	#end of setLoggerPolicy

# end of LoggerStub




#
# constants for ecell
#


# boolean constants
TRUE  = 1
FALSE = 0

# FullPN field numbers
TYPE       = 0
SYSTEMPATH = 1
ID         = 2
PROPERTY   = 3

# Entity type numbers
ENTITY     = 1
SUBSTANCE  = 2
REACTOR    = 3
SYSTEM     = 4

ENTITYTYPE_STRING_LIST =\
( 'NONE', 'Entity', 'Substance', 'Reactor', 'System' )


ENTITYTYPE_DICT =\
{
    'Entity'   : ENTITY,
    'Substance': SUBSTANCE,
    'Reactor'  : REACTOR,
    'System'   : SYSTEM
}    


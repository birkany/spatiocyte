import math 
sim = theSimulator.createStepper('SpatiocyteStepper', 'SS')
sim.VoxelRadius = 10e-9 
sim.SearchVacant = 0

theSimulator.rootSystem.StepperID = 'SS'
theSimulator.createEntity('Variable', 'Variable:/:GEOMETRY').Value = 0
theSimulator.createEntity('Variable', 'Variable:/:LENGTHX').Value = 3e-6
theSimulator.createEntity('Variable', 'Variable:/:LENGTHY').Value = 3e-6
theSimulator.createEntity('Variable', 'Variable:/:LENGTHZ').Value = 3e-6
theSimulator.createEntity('Variable', 'Variable:/:VACANT')
theSimulator.createEntity('Variable', 'Variable:/:Vacant').Value = 0
theSimulator.createEntity('Variable', 'Variable:/:A').Value = 3206
#theSimulator.createEntity('Variable', 'Variable:/:tmp').Value = 0
s = theSimulator.createEntity('Variable', 'Variable:/:sA')
s.Value = 0
s.Name = "HD"

logger = theSimulator.createEntity('VisualizationLogProcess', 'Process:/:logger')
logger.VariableReferenceList = [['_', 'Variable:/:Vacant']]
logger.VariableReferenceList = [['_', 'Variable:/:A']]
#logger.VariableReferenceList = [['_', 'Variable:/:tmp']]
logger.VariableReferenceList = [['_', 'Variable:/:Interface']]
logger.LogInterval = 0.01

pop = theSimulator.createEntity('MoleculePopulateProcess', 'Process:/:pop')
pop.VariableReferenceList = [['_', 'Variable:/:A']]
pop.UniformRadiusWidth = 5e-9
pop.UniformRadiusXZ = 0.2e-6

diffuser = theSimulator.createEntity('DiffusionProcess', 'Process:/:diffuseA')
diffuser.VariableReferenceList = [['_', 'Variable:/:A']]
diffuser.D = 1e-12

binder = theSimulator.createEntity('DiffusionInfluencedReactionProcess', 'Process:/:r')
binder.VariableReferenceList = [['_', 'Variable:/:A','-1']]
binder.VariableReferenceList = [['_', 'Variable:/:Vacant','-1']]
binder.VariableReferenceList = [['_', 'Variable:/:sA','1']]
binder.p = 1

fil = theSimulator.createEntity('FilamentProcess', 'Process:/:filam')
fil.VariableReferenceList = [['_', 'Variable:/:Vacant', '-1']]
fil.VariableReferenceList = [['_', 'Variable:/:sA']]
fil.LineX = 0
fil.LineY = 1
fil.LineZ = 0
fil.Autofit = 0

logger = theSimulator.createEntity('IteratingLogProcess', 'Process:/:iter')
logger.VariableReferenceList = [['_', 'Variable:/:sA']]
logger.LogInterval = 1e-3
logger.LogEnd = 0.25
logger.Iterations = 1
logger.FileName = "IterateLogY.csv"

run(0.3)

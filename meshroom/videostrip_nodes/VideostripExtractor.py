# meshroom/videostrip_nodes/VideostripExtractor.py
from meshroom.core import desc

class VideostripExtractor(desc.CommandLineNode):
    category = 'Videostrip'
    documentation = 'Extract SfM-ready frames + metadata for underwater transects.'
    # You can expose plugin version here if desired.

    inputs = [
        desc.File(name='inputVideo', label='Input Video', required=True),
        desc.Folder(name='outputDir', label='Output Directory', required=True),

        desc.ChoiceParam(name='featureType', label='Feature',
                         values=['ORB','AKAZE','SURF','SIFT'], value='ORB'),
        desc.ChoiceParam(name='imageFormat', label='Image Format',
                         values=['png','jpg'], value='png'),

        desc.FloatParam(name='overlapThreshold', label='Overlap Threshold', value=0.8, range=(0.0, 1.0, 0.01)),
        desc.IntParam(name='maxSkipped', label='Max Skipped Frames', value=4, range=(0, 100, 1)),
        desc.BoolParam(name='enhance', label='Enable Default Enhancement', value=False),

        # Advanced: users can supply a YAML config (your loader merges CLI overrides).
        desc.File(name='configFile', label='YAML Config', optional=True),
    ]

    outputs = [
        # Meshroom typically binds outputs to produced files/dirs; we point to the known locations under outputDir
        desc.Folder(name='imagesDir', label='Images Dir'),
        desc.File(name='framesCsv', label='Frames CSV'),
        desc.File(name='summaryYaml', label='Summary YAML'),
        desc.File(name='runLog', label='Run Log'),
    ]

    # Allow overriding program path via environment/Config.json; default assumes it's on PATH
    videostripCli = desc.File(name='videostripCli', label='videostrip_cli', value='videostrip_cli', uid=[0])

    commandLine = r'''{videostripCli}
        --input "{inputVideo}"
        --output "{outputDir}"
        --feature "{featureType}"
        --format "{imageFormat}"
        --conf {overlapThreshold}
        --skip {maxSkipped}
        {enhanceFlag}
        {configFlag}
        '''

    # Expand optional flags
    def processCommandLine(self, node):
        enhanceFlag = '--enhance' if node.enhance.value else ''
        configFlag = f'--config "{node.configFile.value}"' if node.configFile.value else ''
        return self.commandLine.format(
            videostripCli=node.videostripCli.value,
            inputVideo=node.inputVideo.value,
            outputDir=node.outputDir.value,
            featureType=node.featureType.value,
            imageFormat=node.imageFormat.value,
            overlapThreshold=node.overlapThreshold.value,
            maxSkipped=node.maxSkipped.value,
            enhanceFlag=enhanceFlag,
            configFlag=configFlag
        )

    def postProcess(self, node, context):
        outBase = node.outputDir.value
        node.imagesDir.value = f'{outBase}/images'
        node.framesCsv.value = f'{outBase}/frames.csv'
        node.summaryYaml.value = f'{outBase}/summary.yaml'
        node.runLog.value = f'{outBase}/run.log'

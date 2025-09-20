# meshroom/videostrip_nodes/VideostripExtractor.py
from meshroom.core import desc

class VideostripExtractor(desc.CommandLineNode):
    category = 'Videostrip'
    documentation = 'Extract SfM-ready frames + metadata for underwater transects.'

    # ------------------------------
    # Inputs
    # ------------------------------
    inputs = [
        desc.File(name='inputVideo', label='Input Video', required=True),
        desc.Folder(name='outputDir', label='Output Directory', required=True),

        desc.ChoiceParam(
            name='featureType', label='Feature',
            values=['ORB', 'AKAZE', 'SURF', 'SIFT'], value='ORB'
        ),
        desc.ChoiceParam(
            name='imageFormat', label='Image Format',
            values=['png', 'jpg'], value='png'
        ),

        desc.FloatParam(
            name='overlapThreshold', label='Overlap Threshold',
            value=0.8, range=(0.0, 1.0, 0.01)
        ),
        desc.IntParam(
            name='maxSkipped', label='Max Skipped Frames',
            value=8, range=(0, 100, 1)
        ),
        # When true, we add --enhance (default GrayWorld+CLAHE in your CLI)
        desc.BoolParam(name='enhance', label='Enable Default Enhancement', value=False),

        # Advanced: full YAML config (merged by your loader with CLI overrides)
        desc.File(name='configFile', label='YAML Config', optional=True),

        # Path to the executable; default relies on PATH/config.json
        # group=None prevents "--videostripCli ..." from being appended to {allParams}
        desc.StringParam(name='videostripCli', label='videostrip_cli',
                         value='videostrip_cli', uid=[0], group=None),
    ]

    # ------------------------------
    # Outputs
    # ------------------------------
    outputs = [
        desc.Folder(name='imagesDir', label='Images Dir'),
        desc.File(name='framesCsv', label='Frames CSV'),
        desc.File(name='summaryYaml', label='Summary YAML'),
        desc.File(name='runLog', label='Run Log'),
    ]

    # Optional hints for schedulers (not required)
    # cpu = desc.Level.NORMAL
    # ram = 2 * 1024  # MB

    # We build the exact command string here.
    # Meshroom calls this hook on CommandLineNode/AVCommandLineNode.
    def buildCommandLine(self, chunk):
        n = chunk.node
        # TODO: at this point we could add high level validator to fail early instead of videostrip_cli 
        base = [
            n.videostripCli.value,
            '--input', n.inputVideo.value,
            '--output', n.outputDir.value,
            '--feature', n.featureType.value,
            '--format', n.imageFormat.value,
            '--conf', str(n.overlapThreshold.value),
            '--skip', str(n.maxSkipped.value),
        ]

        if n.enhance.value:
            base.append('--enhance')

        if n.configFile.value:
            base += ['--config', n.configFile.value]

        # Return a single string; Meshroom will handle logging & execution.
        return ' '.join(f'"{x}"' if ' ' in x else x for x in base)

    # After attributes update / invalidation, set outputs derived from outputDir.
    # This hook is part of the CommandLineNode API.
    def postUpdate(self, node):
        outBase = node.outputDir.value
        node.imagesDir.value = f'{outBase}/images'
        node.framesCsv.value = f'{outBase}/frames.csv'
        node.summaryYaml.value = f'{outBase}/summary.yaml'
        node.runLog.value = f'{outBase}/run.log'

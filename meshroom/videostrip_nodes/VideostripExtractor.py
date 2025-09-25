# meshroom/videostrip_nodes/VideostripExtractor.py
from meshroom.core import desc

class VideostripExtractor(desc.CommandLineNode):
    category = 'Videostrip'
    documentation = 'Extract SfM-ready frames + metadata for underwater and aerial transects.'

    # ------------------------------
    # Inputs
    # ------------------------------
    inputs = [
        # NOTE: desc.File require: name, label, description, value
        desc.File(
            name='inputVideo',
            label='Input Video',
            description='Path to the input video file.',
            value=''
        ),
        desc.File(
            name="outputDir",
            label="Output Folder",
            description="Path to base folder for extracted frames and metadata.",
            value="",
        ),
        desc.ChoiceParam(
            name='featureType',
            label='Feature',
            description='Feature detector for keyframe extraction (maps to --feature).',
            values=['ORB', 'AKAZE', 'SURF', 'SIFT'],
            value='ORB'
        ),
        desc.ChoiceParam(
            name='imageFormat',
            label='Image Format',
            description='Image format for exported frames (maps to --format).',
            values=['png', 'jpg'],
            value='png'
        ),

        desc.FloatParam(
            name='overlapThreshold',
            label='Overlap Threshold',
            description='Frame selection confidence/overlap threshold (maps to --conf).',
            value=0.8,
            range=(0.0, 1.0, 0.01)
        ),
        desc.IntParam(
            name='maxSkipped',
            label='Max Skipped Frames',
            description='Maximum consecutive frames to skip between exports (maps to --skip).',
            value=4,
            range=(0, 100, 1)
        ),

        desc.BoolParam(
            name='enhance',
            label='Enable Default Enhancement',
            description='Enable default GrayWorld + CLAHE enhancement (maps to --enhance).',
            value=False
        ),

        # Advanced YAML config merged by your loader with CLI overrides.
        desc.File(
            name='configFile',
            label='YAML Config',
            description='Optional YAML config (supports enhance: and feature_normalization: blocks).',
            value='',
            advanced=True
        ),

        # Path to executable; keep out of {allParams} by using group=None.
        desc.StringParam(
            name='videostripCli',
            label='videostrip_cli',
            description='Path or name of the videostrip CLI executable.',
            value='videostrip_cli',
            group=None
        ),
    ]

    # ------------------------------
    # Outputs
    # ------------------------------
    # TODO: use as output {nodeCacheFolder}
    outputs = [
        desc.File(
            name='imagesDir',
            label='Images Dir',
            description='Directory containing exported frames.',
            value=''
        ),
        desc.File(
            name='framesCsv',
            label='Frames CSV',
            description='CSV with per-frame metadata.',
            value=''
        ),
        desc.File(
            name='summaryYaml',
            label='Summary YAML',
            description='YAML summary with schema_version, config snapshot and run metadata.',
            value=''
        ),
        desc.File(
            name='runLog',
            label='Run Log',
            description='Execution log captured by videostrip_cli.',
            value=''
        ),
    ]

    def buildCommandLine(self, chunk):
        n = chunk.node
        cmd = [
            n.videostripCli.value,
            '--input', n.inputVideo.value,
            '--output', n.outputDir.value,
            '--feature', n.featureType.value,
            '--format', n.imageFormat.value,
            '--conf', str(n.overlapThreshold.value),
            '--skip', str(n.maxSkipped.value),
        ]
        if n.enhance.value:
            cmd.append('--enhance')
        if n.configFile.value:
            cmd += ['--config', n.configFile.value]

        # Quote only args containing spaces
        return ' '.join(f'"{x}"' if ' ' in x else x for x in cmd)

    def postUpdate(self, node):
        outBase = node.outputDir.value
        # // Debug: show the attributes from the node
        if outBase:
            node.imagesDir.value   = f'{outBase}/images'
            node.framesCsv.value   = f'{outBase}/frames.csv'
            node.summaryYaml.value = f'{outBase}/summary.yaml'
            node.runLog.value      = f'{outBase}/run.log'

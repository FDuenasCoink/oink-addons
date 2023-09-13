{
    "targets": [{
        "target_name": "oink-addons",
        'cflags!': [
            '-fno-exceptions'
        ],
        'cflags_cc!': [
            '-fno-exceptions'
        ],
        'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
        },
        'msvs_settings': {
            'VCCLCompilerTool': {
                'ExceptionHandling': 1
            }
        },
        "sources": [
            "src/main.cpp",
            "src/pelicano/PelicanoControl.cpp",
            "src/pelicano/StateMachine.cpp",
            "src/pelicano/ValidatorPelicano.cpp",
            "src/pelicano/Pelicano.cpp",
            "src/azkoyen/AzkoyenControl.cpp",
            "src/azkoyen/StateMachine.cpp",
            "src/azkoyen/ValidatorAzkoyen.cpp",
            "src/azkoyen/Azkoyen.cpp",
            "src/dispenser/Dispenser.cpp",
            "src/dispenser/DispenserControl.cpp",
            "src/dispenser/DispenserWrapper.cpp",
            "src/dispenser/StateMachine.cpp",
            "src/nv10/NV10Control.cpp",
            "src/nv10/NV10Wrapper.cpp",
            "src/nv10/StateMachine.cpp",
            "src/nv10/ValidatorNV10.cpp",
        ],
        'include_dirs': [
            "<!(node -p \"require('node-addon-api').include_dir\")",
            "src/spdlog/include"
        ],
        'libraries': [],
        'defines': ['NAPI_CPP_EXCEPTIONS'],
        'conditions': [
            ['OS=="mac"', {
            'xcode_settings': {
              'OTHER_CFLAGS': [
                '-arch x86_64',
                '-arch arm64'
              ],
              'OTHER_LDFLAGS': [
                '-framework', 'CoreFoundation',
                '-framework', 'IOKit',
                '-arch x86_64',
                '-arch arm64'
              ],
              'SDKROOT': 'macosx',
              'MACOSX_DEPLOYMENT_TARGET': '10.7'
            }
          }],
        ]
    }]
}
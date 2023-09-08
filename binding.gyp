{
    "targets": [{
        "target_name": "pelicano",
        'cflags!': [
            '-fno-exceptions'
        ],
        'cflags_cc!': [
            '-fno-exceptions'
        ],
        'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
        },
        "sources": [
            "src/main.cpp",
            "src/PelicanoControl.cpp",
            "src/StateMachine.cpp",
            "src/ValidatorPelicano.cpp",
            "src/Pelicano.cpp",
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")",
            "src/spdlog/include"
        ],
        'libraries': [],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_CPP_EXCEPTIONS' ],
    }]
}
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
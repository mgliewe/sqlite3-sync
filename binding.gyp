{
    # NOTE: 'module_name' and 'module_path' come from the 'binary' property in package.json
    # node-pre-gyp handles passing them down to node-gyp when you build from source
    "targets": [
        {
            "target_name": "<(module_name)",
            "sources": [ 
                "src/addon.cpp",  
                "src/DatabaseObject.cpp", 
                "src/StatementObject.cpp",
                "src/sqlite.cpp"           
            ],
            "include_dirs": [
                "<!(node -e \"require('node-addon-api').include\")"
            ],
            "libraries": [
                "-lsqlite3"
            ],
            'product_dir': '<(module_path)',
            "xcode_settings": {
                "MACOSX_DEPLOYMENT_TARGET":"10.9"
            }
        }
    ]
}

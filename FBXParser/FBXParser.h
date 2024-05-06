#pragma once

#include "Scene.h"

class FBXParser
{
public:
    FBXParser();
    ~FBXParser();

    int Run(int argc, char* argv[]);

private:
    int Parse(const std::string& filepath);
    bool ImportFbxScene(const std::string& filepath);
    bool ParseFbxScene(const std::string& filepath);

    int Save();

    Scene _scene;

    FbxScene* _fbxScene;
    FbxManager* _fbxManager;
};

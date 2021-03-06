#include "ModelLoader.hpp"

std::map<std::string, Model> ModelLoader::m_models;

void ModelLoader::createModel(std::string modelName, std::string modelPath) {
    std::string objFile = "assets/models/" + modelPath + "/" + modelName + ".obj";
    std::string mtlFile = "assets/models/" + modelPath + "/" + modelName + ".mtl";

    // open mtl file
    std::map<std::string, Material> materials;
    std::ifstream file_mtl(mtlFile, std::ios::in);
    if (!file_mtl.good()) {
        // model does'ent have mtl file
        // std::cout << "WARNING: can't read file \"" << mtlFile << "\"" << std::endl;
    } else {
        Material material;

        // loop troutgh all file lines
        std::string selectedMaterialName = SAH_NULL_STR;
        std::string materialAttrib;
        std::string line;
        while(getline(file_mtl, line)) {
            std::stringstream ss(line);

            ss >> materialAttrib; // get first word
            if (materialAttrib == "newmtl") {
                if (selectedMaterialName != SAH_NULL_STR) {
                    // new material
                    // save last material
                    materials[selectedMaterialName] = material;
                }
                // set new materail name
                ss >> selectedMaterialName;
            } else if (materialAttrib == "Kd") {
                // get defuse color
                glm::vec4 defuse;
                ss >> defuse.r>> defuse.g >> defuse.b;
                defuse.a = 1.0f;

                // set defuse color to materail
                material.setDefuse(defuse);
            } else if (materialAttrib == "map_Kd") {
                // get defuse map
                std::string defiseMapFilePath;
                ss >> defiseMapFilePath;

                // create texture map
                unsigned int textureMap = TextureLoader::createTexture(
                    selectedMaterialName,
                    "assets/models/" + modelName + defiseMapFilePath,
                    false
                );

                // set defuse map
                material.setDefuseMap(textureMap);
            }
        }

        // save last material
        materials[selectedMaterialName] = material;

    }

    // open obj file
    std::ifstream file_obj(objFile, std::ios::in);
    if (!file_obj.good()) {
        std::cout << "WARNING: can't read file \"" << objFile << "\"" << std::endl;
        std::terminate();
    } else {
        // instinitiate model object
        Model model = Model();
        int nbrIndecies = 0;

        // vertex attribs
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        std::string vertexAttrib;

        // loop troutgh all file lines
        std::string meshGroupeMaterialName = SAH_NULL_STR;
        bool newMeshGroup = false;
        std::string line;
        unsigned int lineNbr = 0;
        while(getline(file_obj, line)) {
            std::stringstream ss(line);
            lineNbr++;

            ss >> vertexAttrib; // get first word
            if (vertexAttrib == "v") {
                // vertex position
                glm::vec3 position;

                // get float values from sstream
                ss >> position.x >> position.y >> position.z;

                // push back to uvs vector
                positions.push_back(position);
            } else if (vertexAttrib == "vt") {
                    // vertex uv coordenates
                    glm::vec2 uv;

                    // get float values from sstream
                    ss >> uv.x >> uv.y;

                    // push back to uvs vector
                    uvs.push_back(uv);
            } else if (vertexAttrib == "vn") {
                // vertex normal
                glm::vec3 normal;

                // get float values from sstream
                ss >> normal.x >> normal.y >> normal.z;

                // push back to uvs vector
                normals.push_back(normal);
            } else if (vertexAttrib == "usemtl") {
                if (newMeshGroup) {
                    newMeshGroup = false;

                    // get mesh group indecies
                    unsigned int indecies[nbrIndecies];
                    for (int k = 0; k < nbrIndecies; k++) {
                        indecies[k] = k;
                    };

                    // model mesh group load vertices
                    if (meshGroupeMaterialName != SAH_NULL_STR) {
                        model.loadVertices(indecies, nbrIndecies, materials[meshGroupeMaterialName]);
                    } else {
                        model.loadVertices(indecies, nbrIndecies, Material());
                    }

                    // reset indecies nbr
                    nbrIndecies = 0;
                }

                // set mesh group material name
                ss >> meshGroupeMaterialName;

            } else if (vertexAttrib == "f") {
                // reset meshGroup bool
                newMeshGroup = true;

                // face attrib
                std::string faceAttrib;

                // inverse faces to clock-wize
                std::string correctFaceOrderList[3];
                std::string correctFaceOrder;
                for (int w = 0; w < 3; w++) {
                    ss >> faceAttrib;
                    correctFaceOrderList[w] = faceAttrib + " ";
                };
                for (int w = 2; w >= 0; w--) {
                    correctFaceOrder += correctFaceOrderList[w];
                }
                std::istringstream ssc(correctFaceOrder);

                // get vertex attribs indecies (pos/uv/norm)
                for (int i = 0; i < 3; i++) { // 3 vertecies in a face
                    ssc >> faceAttrib;

                    // add an index
                    nbrIndecies++;

                    // get vertex attribs indecies
                    std::istringstream ssa(faceAttrib);
                    std::string token;
                    int vertexAttribIndex[3];
                    int index;
                    for (int j = 0; j < 3; j++) { // 3 attribs in a vertex
                        std::getline(ssa, token, '/');
                        // token does'nt exist
                        if (token == "") {
                            std::string errorFaceAttrib;
                            switch (j) {
                                case 0:
                                    errorFaceAttrib = "Position";
                                    index = 1;
                                    positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

                                    // warn about error
                                    std::cout << "ERROR: model \"" << objFile << "\" dosen't have " << errorFaceAttrib << " at line " << lineNbr << '\n';
                                    break;
                                case 1:
                                    errorFaceAttrib = "UV coordenates";
                                    index = 2;
                                    uvs.push_back(glm::vec2(0.0f, 0.0f));

                                    // warn about error
                                    if (LOG_ERRORS) {
                                        std::cout << "WARNING: model \"" << objFile << "\" dosen't have " << errorFaceAttrib << " at line " << lineNbr << '\n';
                                    }
                                    break;
                                case 2:
                                    errorFaceAttrib = "Normal";
                                    index = 3;
                                    normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

                                    // warn about error
                                    std::cout << "WARNING: model \"" << objFile << "\" dosen't have " << errorFaceAttrib << " at line " << lineNbr << '\n';
                                    break;
                            }

                        } else {
                            std::istringstream iss(token);
                            iss >> index;
                        }

                        vertexAttribIndex[j] = index-1;
                    }

                    // model add vertex
                    model.addVertex(
                        positions[vertexAttribIndex[0]],
                        uvs[vertexAttribIndex[1]],
                        normals[vertexAttribIndex[2]],
                        glm::vec3(1.0f, 1.0f, 1.0f)
                    );
                }
            }
        }

        // get mesh group indecies
        unsigned int indecies[nbrIndecies];
        for (unsigned int k = 0; k < nbrIndecies; k++) {
            indecies[k] = k;
        };

        if (meshGroupeMaterialName != SAH_NULL_STR) {
            // model mesh group load vertices
            model.loadVertices(indecies, nbrIndecies, materials[meshGroupeMaterialName]);
        } else {
            // model mesh group load vertices
            model.loadVertices(indecies, nbrIndecies, Material());
        }

        // add it to models map
        m_models[modelName] = model;
    }

};

Model ModelLoader::getModel(std::string modelName) {
    return m_models[modelName];
};

void ModelLoader::deleteModel() {

};

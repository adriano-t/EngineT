
#include "shader_manager.h"
#include "string_utils.h"
#include "shader_mesh.h"
#include "shader_lighting.h"


namespace EngineT
{
    ShaderManager::ShaderManager() {}

    ShaderManager::~ShaderManager() {}

     

    GLuint ShaderManager::CreateProgram(const string& vertexShader, const string& fragmentShader, const string& geometryShader)
    {
        GLuint shaderProg = glCreateProgram();

        if(shaderProg == 0) {
            errors.push_back("Error generating shader program " + __LINE__);
            return 0;
        }

        GLuint vertex = CreateShader(GL_VERTEX_SHADER, vertexShader);
        if(vertex == 0) return 0;
        glAttachShader(shaderProg, vertex);

        GLuint fragment = CreateShader(GL_FRAGMENT_SHADER, fragmentShader);
        if(fragment == 0) return 0;
        glAttachShader(shaderProg, fragment);

        GLuint geometry;
        if(geometryShader.length() > 0){
            geometry = CreateShader(GL_GEOMETRY_SHADER, geometryShader);
            if(geometry == 0) return 0;
            glAttachShader(shaderProg, geometry);
        }


        GLint success = 0;
        GLchar errorLog[1024] = {0};

        glLinkProgram(shaderProg);

        glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);
        if(success == 0) {
            glGetProgramInfoLog(shaderProg, sizeof(errorLog), NULL, errorLog);
            errors.push_back("Error linking shader program:" + (string) errorLog);
            return 0;
        }

        glValidateProgram(shaderProg);
        glGetProgramiv(shaderProg, GL_VALIDATE_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(shaderProg, sizeof(errorLog), NULL, errorLog);
            errors.push_back("Invalid shader program:" + (string) errorLog);
            return 0;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(geometryShader.length() > 0){
            glDeleteShader(geometry);
        }


        return shaderProg;
    } 

    GLuint ShaderManager::CreateShader(GLenum shaderType, const string& shaderText)
    {
        GLuint shaderObj = glCreateShader(shaderType);

        if(shaderObj == 0) {
            errors.push_back("Error creating " + ShaderTypeString(shaderType) + " shader");
            return 0;
        }

        const GLchar* p[1];
        p[0] = shaderText.c_str();
        GLint lengths[1];
        lengths[0] = strlen(shaderText.c_str());
        glShaderSource(shaderObj, 1, p, lengths);

        glCompileShader(shaderObj);

        GLint success;
        glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);

        if(!success) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(shaderObj, 1024, NULL, infoLog);
            errors.push_back("Error compiling " + ShaderTypeString(shaderType) + " shader: " + (string) infoLog);
            return 0;
        }

        return shaderObj;
    }




    GLint ShaderManager::GetUniformLoc(GLuint shaderProg, const char* uniformName)
    {
        GLint location = glGetUniformLocation(shaderProg, uniformName);

        if(location == 0xFFFFFFFF) {
            errors.push_back("invalid uniform location:" + ((string) uniformName));
        }

        return location;
    }


    unordered_map<string, GLint> ShaderManager::GetAllUniforms(GLuint shaderProg)
    {
        unordered_map<string, GLint> locations;

        GLint count;
        glGetProgramiv(shaderProg, GL_ACTIVE_UNIFORMS, &count);

        cout << "<------ SHADER uniforms --------->" << endl;
        cout << "uniforms count: " << count << endl;
        std::vector<GLchar> nameData(256);
        for(int unif = 0; unif < count; ++unif)
        {
            GLint arraySize = 0;
            GLenum type = 0;
            GLsizei actualLength = 0;
            glGetActiveUniform(shaderProg, unif, nameData.size(), &actualLength, &arraySize, &type, &nameData[0]);
            string name((char*) &nameData[0], actualLength);

            cout << unif << ") " << name << endl;
            GLint location = glGetUniformLocation(shaderProg, name.c_str());

            if(location == 0xFFFFFFFF) {
                errors.push_back("invalid uniform location:" + name);
            }
            else{
                locations.insert(pair<string, GLint>(name, location));
            }
        }

        cout << " " << endl;
        return std::move(locations);
    }



    string ShaderManager::ShaderTypeString(GLuint shaderType)
    {
        switch(shaderType){
        case GL_VERTEX_SHADER:
            return "vertex";
            break;
        case GL_FRAGMENT_SHADER:
            return "fragment";
            break;
        case GL_GEOMETRY_SHADER:
            return "geometry";
            break;
        default:
            return "unknown";
            break;
        }
    }


    void ShaderManager::PrintErrors()
    {
        if(errors.size() == 0){
            //Debug::SetColor();
            cout << "No errors in shader manager" << endl;
            return;
        }
        for(auto it = errors.begin(); it < errors.end(); it++){
            cout << *it << endl;
        }
        errors.clear();
    }

    string ShaderManager::ReadShaderFile(const char* filename)
    {
        ifstream file;
        file.open(filename);
        if(!file) {
            cout << "Impossible to open shader file: " << filename << endl;
            return "";
        }
        stringstream stream;
        stream << file.rdbuf();
        file.close();

        return stream.str();
    }

    Shader* ShaderManager::Load(const string filename)
    {
        string path = Engine.dataPath + "shaders/includes/";

        Shader* newShader = nullptr;
        ifstream file;
        file.open(filename);
        if(!file.is_open())
        {
            cout << "Impossible to open shader file: " << filename << endl;
            return newShader;
        }

        vector<string> vertexInput;
        vector<string> geometryInput;
        vector<string> fragmentInput;
       
        //read sections
        string line, trimLine;
        string currentSection;
        int  i = 0;
        while(getline(file, line))
        {
            trimLine = StringUtils::Trim(line);
            i++;
            if(trimLine.length() > 0)
            {

                if(line[0] == '{' || line[0] == '}')
                    continue;
                else if(trimLine == "Header")
                    currentSection = trimLine;
                else if(trimLine == "Vertex")
                    currentSection = trimLine;
                else if(trimLine == "Geometry")
                    currentSection = trimLine;
                else if(trimLine == "Fragment")
                    currentSection = trimLine;
                else
                { 
                    if(currentSection == "Header")
                    {
                        if(trimLine[0] != '{' && trimLine[0] != '}') 
                            header.push_back(trimLine);
                    }
                    else if(currentSection == "Vertex")
                        vertexInput.push_back(line);
                    else if(currentSection == "Geometry")
                        geometryInput.push_back(line);
                    else if(currentSection == "Fragment")
                        fragmentInput.push_back(line);
                    else
                        errors.push_back("Error loading shader: " + filename + "\n at line " + to_string(i));
                }
            }
        }

        file.close();
         
        //map header
        for(string line : header)
        {
            int pos = line.find_first_of(':');
            string key = line.substr(0, pos );
            string value = StringUtils::Trim(line.substr(pos + 1, line.length() - pos));
            if(key.length() == 0 || value.length() == 0)
            {
                errors.push_back("key or value empty " + key + ":" + "value");
                continue;
            }
            headerMap[key] = value;
            defines.insert(std::move(key + ":" + value));
        }

        cout << "header defines "  << " " << defines.size() << endl;

        //=====================
        //= Parse header
        //=====================

        //name
        string name;
        if(HeaderExits("Name"))
            name = headerMap["Name"];
        else
            errors.push_back("Header Error: Missing 'Name' parameter");

        //version
        string version = "#version 330";
        if(HeaderExits("Version"))
            version = "#version " + headerMap["Version"];
        vertex.push_back(version);
        fragment.push_back(version);

        //=====================
        //= Parse content
        //=====================
        if(HeaderEnabled("Mesh"))
        {
            cout << "Loading mesh shader" << endl;
            ParseShader(vertexInput, vertex, path + "mesh.vertex");
            ParseShader(fragmentInput, fragment, path + "mesh.fragment");
            newShader = new ShaderMesh();
        }
        else if(HeaderEnabled("Lighting"))
        {
            int maxPointLights = 2;
            int maxSpotLights = 2;
            fragment.push_back("const int MAX_POINT_LIGHTS = " + to_string(maxPointLights) + ";");
            fragment.push_back("const int MAX_SPOT_LIGHTS = " + to_string(maxSpotLights) + ";");

            ParseShader(vertexInput, vertex, path + "lighting.vertex");
            ParseShader(fragmentInput, fragment, path + "lighting.fragment");
            newShader = new ShaderLighting();
            ((ShaderLighting*) newShader)->hasSpecular = HeaderEnabled("Specular");
            ((ShaderLighting*) newShader)->hasNnormalmap = HeaderEnabled("Normalmap");
            cout << "Created ShaderLighting..." << endl;
        } 
        else
        {
            cout << "Creating normal shader" << endl;
        }

        SaveToFile(path + "DEBUG_" + name + ".vertex", vertex);
        SaveToFile(path + "DEBUG_" + name + ".fragment", fragment);

        string sVertex;
        for(string line : vertex)
            sVertex += line + '\n';

        string sFragment;
        for(string line : fragment)
            sFragment += line + '\n';
        
        string sGeometry;


        //clear resources
        header.clear();
        headerMap.clear();
        vertex.clear();
        geometry.clear();
        fragment.clear();
        defines.clear();

        //=====================
        //= Compile
        //=====================
        if(newShader->CreateProgram(sVertex, sFragment, sGeometry))
        {
            cout << name << " compiled." << endl;
            return newShader;
        }
        else
        {
            cout << name << " not compiled." << endl;
            delete newShader;
        }
        return nullptr;

    }


    void ShaderManager::ParseShader(vector<string>& input, vector<string>& output, const string filename)
    {
        ifstream file;
        file.open(filename);
        if(!file.is_open())
        {
            cout << "Impossible to parse vertex shader file: " << filename << endl; 
            return;
        }
        cout << "parsing vertex shader " << filename << endl;

        //read sections
        string line, trimLine;
        int  i = 0;
        bool skip;
        while(getline(file, line))
        {
            trimLine = StringUtils::Trim(line);
            if(trimLine.find("#if") != trimLine.npos)
            {
                if(defines.find(trimLine.substr(4)) == defines.end())
                    skip = true;
            }
            else if(trimLine.find("#endif") != trimLine.npos)
            {
                skip = false;
            }
            else if(!skip)
            {
                //add custom lines
                if(trimLine.find("void main") != trimLine.npos)
                {
                    for(string line : input)
                        output.push_back(line);
                }

                output.push_back(std::move(line));
            }
        }
    }

   
    bool ShaderManager::SaveToFile(const string filename, vector<string>& lines)
    {
        std::ofstream file;
        file.open(filename);
        if(!file.is_open())
        {
            cout << "Impossible to save shader file: " << filename << endl;
            return false;
        }

        for(string line : lines)
            file << line << endl;

        file.close();

        return true;

    }
}
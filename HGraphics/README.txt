a. Build and run solution file.
b. Every item for assignment is finished.
c. File Location: 
-Source files: (ProjectDir)/src
   - main.cpp 
      * int main() 
      * void framebuffer_size_callback(GLFWwindow* widow, int width, int height)
   - mesh.cpp
      * Mesh()
      * ~Mesh()
      * GLfloat* getVertexBuffer()
      * GLfloat* getVertexNormals()
      * GLfloat* getVertexUVs()
      * GLfloat* getVertexNormalsForDisplay()
      * unsigned int getVertexBufferSize()
      * unsigned int getVertexCount()
      * unsigned int getVertexNormalCount()
      * GLuint* getIndexBuffer()
      * unsigned int getIndexBufferSize()
      * unsigned int getTriangleCount()
      * glm::vec3 getModelScale()
      * glm::vec3 getModelCentroid()
      * glm::vec3 getCentroidVector(glm::vec3 vVertex)
      * loat getModelScaleRatio()
      * virtual void render(int Flag = 0) const
      * void setupMesh()
      * void setupVNormalMesh()
      * void setupFNormalMesh()
      * GLfloat& getNormalLength()
      * void setNormalLength(GLfloat nLength)
      * void initData()
      * int calcVertexNormals(GLboolean bFlipNormals = false)
      * void calcVertexNormalsForDisplay(GLboolean bFlipNormals = false)
      * int calcUVs(Mesh::UVType uvType = UVType::PLANAR_UV)
      * glm::vec2 calcCubeMap(glm::vec3 vEntity)

   - OBJManager.cpp
      * OBJManager()
      * virtual ~OBJManager()
      * double ReadOBJFile(std::string filepath, Mesh* pMesh, ReadMethod r = ReadMethod::LINE_BY_LINE, GLboolean bFlipNormals = false)
      * GLuint loadOBJFile(std::string fileName, std::string modelName, bool bNormalFlag)
      * void initData()
      * Mesh* GetMesh(const std::string& name)
      * LineMesh* GetLineMesh(const std::string& name)
      * void setupSphere(const std::string& modelName)
      * void setupOrbitLine(const std::string& name, float radius)
      * int ReadOBJFile_LineByLine(std::string filepath)
      * int ReadOBJFile_BlockIO(std::string filepath)
      * void ParseOBJRecord(char* buffer, glm::vec3& min, glm::vec3& max)

   - scene.cpp
      * Scene()
      * Scene(int windowWidth, int windowHeight)
      * virtual ~Scene()
      * virtual int Init()
      * virtual void LoadAllShaders()
      * virtual int Display()
      * virtual int preRender()
      * virtual int Render()
      * virtual int postRender()
      * virtual void CleanUp()
      * virtual void SetupImGUI(GLFWwindow* pWwindow)
      * virtual void RenderImGUI()
      * virtual void ProcessInput(GLFWwindow* pWwindow, double dt)

   - shader.cpp
      * Shader(const char* vertex_file_path, const char* fragment_file_path, const char* geometryPath = nullptr)
      * void use()
      * int getHandle()
      * void SetUniform(const std::string& name, GLboolean value) const
      * void SetUniform(const std::string& name, GLint value) const
      * void SetUniform(const std::string& name, GLuint value) const
      * void SetUniform(const std::string& name, GLfloat value) const
      * void SetUniform(const std::string& name, const GLdouble value) const
      * void SetUniform(const std::string& name, const glm::vec3& value) const
      * void SetUniform(const std::string& name, GLfloat x, GLfloat y, GLfloat z) const
      * void SetUniform(const std::string& name, const glm::vec4& value) const
      * void SetUniform(const std::string& name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const
      * void SetUniform(const std::string& name, const glm::mat4& mat) const
      * void SetUniform(const std::string& name, const glm::mat3& mat) const

   - LineMesh.cpp
      * LineMesh()
      * ~LineMesh()
      * void render(int bFlag = 0) const override
      * void setupLineMesh()

   -simpleScene.cpp
      * SimpleScene() = default
      * SimpleScene(int windowWidth, int windowHeight)
      * virtual ~SimpleScene()
      * int Init() override
      * int Render() override
      * int postRender() override
      * void SetupImGUI(GLFWwindow* pWwindow) override
      * void RenderImGUI() override
      * void ProcessInput(GLFWwindow* pWwindow, double dt) override
-Header files: (ProjectDir)/include
   - Camera.h 
   - LineMesh.h
   - mesh.h 
   - OBJManager.h 
   - shader.hpp
   - simpleScene.h 
-Shader files: (ProjectDir)/shader
   - normalShader.vert
   - normalShader.frag
   - shader.vert
   - shader.frag
d. Tested on Window10 machine with GTX1660ti & OpenGL4.6
e. The number of hours you spent : 6H per week. Worked for 3 weeks.
f. WASD for camera movement. QE for camera yaw.
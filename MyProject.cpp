// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

#define NUM_VIEWS 3
#define NUM_CORNERS 4
#define GOAL_SCORE 7
#define DEBOUNCE_THRESHOLD 0.5f
#define NUM_TEXTURES 4
#define TIME_SPEED 1.5f
#define REACTION_TIME 1.5f


//const SkyBoxModel  SkyBoxToLoad1 = { "SkyBoxCube.obj", OBJ, {"skybox/sea/posx.jpg", "skybox/sea/negx.jpg", "skybox/sea/posy.jpg", "skybox/sea/negy.jpg", "skybox/sea/posz.jpg", "skybox/sea/negz.jpg"} };
const SkyBoxModel  SkyBoxToLoad[] = {
    {"SkyBoxCube.obj", OBJ, {"skybox/space/bkg1_right.jpeg", "skybox/space/bkg1_left.jpeg", "skybox/space/bkg1_top.jpeg", "skybox/space/bkg1_bot.jpeg", "skybox/space/bkg1_front.jpeg", "skybox/space/bkg1_back.jpeg"}},
    {"SkyBoxCube.obj", OBJ, {"skybox/cloudy/px.png", "skybox/cloudy/nx.png", "skybox/cloudy/py.png", "skybox/cloudy/ny.png", "skybox/cloudy/pz.png", "skybox/cloudy/nz.png"}}, 
    {"SkyBoxCube.obj", OBJ, {"skybox/superMario/px.png", "skybox/superMario/nx.png", "skybox/superMario/py.png", "skybox/superMario/ny.png", "skybox/superMario/pz.png", "skybox/superMario/nz.png"}},
    {"SkyBoxCube.obj", OBJ, {"skybox/sea/posx.jpg", "skybox/sea/negx.jpg", "skybox/sea/posy.jpg", "skybox/sea/negy.jpg", "skybox/sea/posz.jpg", "skybox/sea/negz.jpg"}}
};


// The uniform buffer object used in this example
struct globalUniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 lightColor;
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec3 ambColor;
    alignas(16) glm::vec4 coneInOutDecayExp;
    alignas(16) glm::vec3 spotPosition1;
    alignas(16) glm::vec3 spotPosition2;
    alignas(16) glm::vec3 spotDirection;
    alignas(16) glm::vec3 eyePos;
    alignas(8)  glm::vec2 selector;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
};

struct SkyBoxUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};


struct Point {
    float x;
    float y;
    float vx;
    float vy;
};

// MAIN !
class MyProject : public BaseProject {
protected:
    // Here you list all the Vulkan objects you need:

    // Descriptor Layouts [what will be passed to the shaders]
    DescriptorSetLayout DSLglobal;
    DescriptorSetLayout DSLobj;
    //DescriptorSetLayout DSL_SB;

    // Pipelines [Shader couples]
    Pipeline P1;
    //Pipeline P_SB;

    // SkyBox
    DescriptorSetLayout DSL_SB;
    Pipeline P_SB;


    //SkyBox
    ModelData M_SB;
    TextureData T_SB[NUM_TEXTURES];
    DescriptorSetSkyBox DS_SB[NUM_TEXTURES];


    // Models, textures and Descriptors (values assigned to the uniforms)
    Model M_Table;
    Texture T_Table[NUM_TEXTURES];
    DescriptorSet DS_Table[NUM_TEXTURES];

    //Puck
    Model M_Puck;
    Texture T_Puck;
    DescriptorSet DS_Puck;    
    //Paddle
    Model M_Paddle;
    Texture T_LeftPaddle;
    Texture T_RightPaddle;
    DescriptorSet DS_LeftPaddle;    // instance DSLobj
    DescriptorSet DS_RightPaddle;    // instance DSLobj

    //Start Screens
    Model M_Panel;
    Texture T_StartScreen;
    Texture T_Settings;
    Texture T_Pause;
    DescriptorSet DS_StartScreen;
    DescriptorSet DS_Settings;
    DescriptorSet DS_Pause;

    //Points
    Model M_Numbers[GOAL_SCORE];
    Texture T_LNumbers;
    Texture T_RNumbers;
    DescriptorSet DS_LNumbers[GOAL_SCORE];
    DescriptorSet DS_RNumbers[GOAL_SCORE];
    
    //Win Screen
    Model M_BlueWin;
    Model M_RedWin;
    Model M_Reset;
    Texture T_Win;
    DescriptorSet DS_BlueWin;
    DescriptorSet DS_RedWin;
    DescriptorSet DS_Reset;
    
    DescriptorSet DS_global;

    //Temp variable
    DescriptorSet* toBind; 

    //Other variables
    int leftPlayerScore = 0;
    int rightPlayerScore = 0;

    //Assumption: the table is centered in (0, 0)
    float halfTableLength = 1.7428f / 2;
    float halfTableWidth = 0.451f;
    float halfWholeTableHeight = 0.05f;
    float halfSideHeight = 0.018f;
    float halfTableHeight = halfWholeTableHeight - halfSideHeight;
    float cornerCircleRadius = halfTableWidth / 2;
    //Because of the table simmetry, only one of the 4 centers is required in calculations if modulus is used
    Point cornerCircleCenter = {
        -halfTableLength + cornerCircleRadius,
        -cornerCircleRadius
    };
    float scoreAreaX = 0.4f;

    float puckRadius = 0.0574f / 2;
    float puckVelocity = 0.65f;
    float initialPuckAngle = glm::radians(60.0f);
    Point puck = { 0.0f, 0.0f, 0.0f, 0.0f };
    bool completelyBounced = true;

    float paddleRadius = 0.07;
    float paddleVelocity = 0.2f;
        
    float winTextAngle = 0;
    
    glm::vec2 sel = glm::vec2(1.0f, 1.0f);
    
    Point lPaddle = { -halfTableLength + paddleRadius, 0.0f, 0.0f, 0.0f };
    Point rPaddle = { halfTableLength - paddleRadius, 0.0f, 0.0f, 0.0f };

    Point lScore = { -halfTableLength / 2, -halfTableWidth - 0.1f };
    Point rScore = { halfTableLength / 2, -halfTableWidth - 0.1f };

    enum difficulties {
        EASY,
        NORMAL,
        HARD
    } difficulty = NORMAL;

    enum states {
        START,
        SETTINGS,
        RESET,
        PLAYING,
        VICTORY,
        PAUSE,
        RESUME
    } state = START;

    enum views {
        ABOVE,
        LEFTPLAYER,
        RIGHTPLAYER,     
    } view = ABOVE;
    views oldView;
    
    enum skyBoxes {
        SPACE,
        CLOUDS,
        TOON,
        MARINE
    } skyBox = SPACE;

    enum players {
        NONE,
        LEFT,
        RIGHT
    } winner = NONE;

    float dt = 0.01f;
    std::chrono::time_point<std::chrono::system_clock> lastTime = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> debounceTime = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> currentTime;
    
    // Here you set the main application parameters
    void setWindowParameters() {
        // window size, titile and initial background
        windowWidth = 1250;
        windowHeight = 900;
        windowTitle = "Air Hockey";
        initialBackgroundColor = {0.0f, 0.5f, 0.0f, 1.0f};
        
        // Descriptor pool sizes
        uniformBlocksInPool = 32;
        texturesInPool = 31;
        setsInPool = 32;
    }
    
    // Here you load and setup all your Vulkan objects
    void localInit() {
        // Descriptor Layouts [what will be passed to the shaders]
        DSLobj.init(this, {                
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
                  });

        DSLglobal.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                  });
            

        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on..
        P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSLglobal, &DSLobj}, VK_COMPARE_OP_LESS);
        
        DSL_SB.init(this, {
                            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
                          });
        P_SB.init(this, "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", {&DSL_SB}, VK_COMPARE_OP_LESS_OR_EQUAL);
        M_SB.init(this, SkyBoxToLoad[0]);
        
        for (int i = 0; i < NUM_TEXTURES; i++) {
            T_SB[i].init(this, SkyBoxToLoad[i]);
            DS_SB[i].init(this, &DSL_SB, {
                            {0, UNIFORM, sizeof(SkyBoxUniformBufferObject), nullptr},
                            {1, TEXTURE, 0, &T_SB[i]}
                        });
        }
        
        // Models, textures and Descriptors (values assigned to the uniforms)
        M_Table.init(this, "models/table.obj");
        for (int i = 0; i < NUM_TEXTURES; i++) {
            T_Table[i].init(this, "textures/airHockey" + std::to_string(i) + ".png");
            DS_Table[i].init(this, &DSLobj, {
                // the second parameter, is a pointer to the Uniform Set Layout of this set
                // the last parameter is an array, with one element per binding of the set.
                // first  elmenet : the binding number
                // second element : UNIFORM or TEXTURE (an enum) depending on the type
                // third  element : only for UNIFORMs, the size of the corresponding C++ object
                // fourth element : only for TEXTUREs, the pointer to the corresponding texture object
                            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                            {1, TEXTURE, 0, &T_Table[i]}
                });
        }
    
        //Puck
        M_Puck.init(this, "models/disk.obj");
        T_Puck.init(this, "textures/disk.png");
        DS_Puck.init(this, &DSLobj, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &T_Puck}
                });

        //Paddle
        M_Paddle.init(this, "models/paddle.obj");
        T_LeftPaddle.init(this, "textures/paddleLeft.png");
        DS_LeftPaddle.init(this, &DSLobj, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &T_LeftPaddle}
                });
        T_RightPaddle.init(this, "textures/paddleRight.png");
        DS_RightPaddle.init(this, &DSLobj, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &T_RightPaddle}
                });
        
        //Start Screen
        M_Panel.init(this, "models/ground.obj");
        T_StartScreen.init(this, "textures/StartScreen.png");
        T_Settings.init(this, "textures/Settings.png");
        T_Pause.init(this, "textures/Pause.png");
        DS_StartScreen.init(this, &DSLobj, {
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
            {1, TEXTURE, 0, &T_StartScreen}
        });
        DS_Settings.init(this, &DSLobj, {
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
            {1, TEXTURE, 0, &T_Settings}
        });
        DS_Pause.init(this, &DSLobj, {
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
            {1, TEXTURE, 0, &T_Pause}
        });

        //Numbers
        /*T_LNumbers.init(this, "textures/leftNumbers.png");
        T_RNumbers.init(this, "textures/rightNumbers.png");
        TODO Do we want to give numbers a different texture?*/
        for (int i = 0; i < GOAL_SCORE; i++) {
            M_Numbers[i].init(this, "models/points/" + std::to_string(i) + ".obj");

            DS_LNumbers[i].init(this, &DSLobj, {
                        {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                        {1, TEXTURE, 0, &T_LeftPaddle}
                });
            DS_RNumbers[i].init(this, &DSLobj, {
                        {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                        {1, TEXTURE, 0, &T_RightPaddle}
                });
        }
        
        
        //Win Screen
        M_BlueWin.init(this, "models/BlueWin.obj"); //Blue Win
        M_RedWin.init(this, "models/RedWin.obj"); //Red Win
        T_Win.init(this, "textures/gold.jpeg");
        DS_BlueWin.init(this, &DSLobj, {
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
            {1, TEXTURE, 0, &T_Win}
        });
        DS_RedWin.init(this, &DSLobj, {
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
            {1, TEXTURE, 0, &T_Win}
        });
        
        M_Reset.init(this, "models/Restart.obj");
        DS_Reset.init(this, &DSLobj, {
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
            {1, TEXTURE, 0, &T_Win}
        });
        

        DS_global.init(this, &DSLglobal, {
                    {0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}
                });      
        
    }
    
    void localResizeInit(){
        P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSLglobal, &DSLobj}, VK_COMPARE_OP_LESS);
        P_SB.init(this, "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", {&DSL_SB}, VK_COMPARE_OP_LESS_OR_EQUAL);
    }
    
    // Here you destroy all the objects you created!
    //TODO Check everything is destroyed properly
    void localCleanup() {
        
        //SkyBox
        for (int i = 0; i < NUM_TEXTURES; i++) {
            DS_SB[i].cleanup();
            T_SB[i].cleanup();        
        }
        M_SB.cleanup();
        P_SB.cleanup();
        DSL_SB.cleanup();
        
        //Table
        for (int i = 0; i < NUM_TEXTURES; i++) {
            DS_Table[i].cleanup();
            T_Table[i].cleanup();
            M_Table.cleanup();
        }       
        
        //Puck
        DS_Puck.cleanup();
        T_Puck.cleanup();
        M_Puck.cleanup();
        
        //Numbers
        for (int i = 0; i < GOAL_SCORE; i++) {
            DS_LNumbers[i].cleanup();
            DS_RNumbers[i].cleanup();
            
            M_Numbers[i].cleanup();
        }
        
        //Paddles
        DS_LeftPaddle.cleanup();
        DS_RightPaddle.cleanup();
        T_LeftPaddle.cleanup();
        T_RightPaddle.cleanup();
        M_Paddle.cleanup();
        

        //Start Screen
        DS_StartScreen.cleanup();
        DS_Settings.cleanup();
        DS_Pause.cleanup();
        T_StartScreen.cleanup();
        T_Settings.cleanup();
        T_Pause.cleanup();
        M_Panel.cleanup();
        
        //Win Text
        DS_BlueWin.cleanup();
        DS_RedWin.cleanup();
        DS_Reset.cleanup();
        T_Win.cleanup();
        M_BlueWin.cleanup();
        M_RedWin.cleanup();
        M_Reset.cleanup();
        
        DS_global.cleanup();

        P1.cleanup();
        
        DSLglobal.cleanup();
        DSLobj.cleanup();              
    }
    
    void localResizeCleanup(){
        P1.cleanup();
        P_SB.cleanup();
    }


    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
                
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        P_SB.graphicsPipeline);

        VkBuffer vertexBuffers_SB[] = {M_SB.vertexBuffer};
        VkDeviceSize offsets_SB[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers_SB, offsets_SB);
        vkCmdBindIndexBuffer(commandBuffer, M_SB.indexBuffer, 0,
                                        VK_INDEX_TYPE_UINT32);
        
        for (int i = 0; i < NUM_TEXTURES; i++)
            if (i == skyBox)
                vkCmdBindDescriptorSets(commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    P_SB.pipelineLayout, 0, 1, &DS_SB[i].descriptorSets[currentImage],
                    0, nullptr);
            
        vkCmdDrawIndexed(commandBuffer,
                                        static_cast<uint32_t>(M_SB.indices.size()), 1, 0, 0, 0);
                
        
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                P1.graphicsPipeline);
        vkCmdBindDescriptorSets(commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
                        0, nullptr);
        
        
       
        VkBuffer vertexBuffers[] = {M_Table.vertexBuffer};
        // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        // property .indexBuffer of models, contains the VkBuffer handle to its index buffer
        vkCmdBindIndexBuffer(commandBuffer, M_Table.indexBuffer, 0,
                                VK_INDEX_TYPE_UINT32);

        for (int i = 0; i < NUM_TEXTURES; i++)
           if (i == skyBox) 
                vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Table[i].descriptorSets[currentImage],
                                0, nullptr);
                                
                // property .indices.size() of models, contains the number of triangles * 3 of the mesh.
        vkCmdDrawIndexed(commandBuffer,
                    static_cast<uint32_t>(M_Table.indices.size()), 1, 0, 0, 0);
                    
                
        VkBuffer vertexBuffers_SC[] = {M_Panel.vertexBuffer};
        VkDeviceSize offsets_SC[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers_SC, offsets_SC);
        vkCmdBindIndexBuffer(commandBuffer, M_Panel.indexBuffer, 0,
                                VK_INDEX_TYPE_UINT32);        
        switch (state) {
            case START:
                toBind = &DS_StartScreen;                
                break;
            case SETTINGS:
                toBind = &DS_Settings;                               
                break;
            case PAUSE:
                toBind = &DS_Pause;            
                break;
            default:
                toBind = NULL; //Random value to be used as Null
        }

        if (toBind != NULL) {
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                P1.pipelineLayout, 1, 1, &toBind->descriptorSets[currentImage],
                0, nullptr);
            vkCmdDrawIndexed(commandBuffer,
                static_cast<uint32_t>(M_Panel.indices.size()), 1, 0, 0, 0);
        }
             
       
        VkBuffer vertexBuffers2[] = {M_Puck.vertexBuffer};
        VkDeviceSize offsets2[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers2, offsets2);
        vkCmdBindIndexBuffer(commandBuffer, M_Puck.indexBuffer, 0,
                                VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        P1.pipelineLayout, 1, 1, &DS_Puck.descriptorSets[currentImage],
                        0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                    static_cast<uint32_t>(M_Puck.indices.size()), 1, 0, 0, 0);



        VkBuffer vertexBuffers3[] = {M_Paddle.vertexBuffer};
        VkDeviceSize offsets3[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers3, offsets3);
        vkCmdBindIndexBuffer(commandBuffer, M_Paddle.indexBuffer, 0,
                                VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        P1.pipelineLayout, 1, 1, &DS_LeftPaddle.descriptorSets[currentImage],
                        0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                    static_cast<uint32_t>(M_Paddle.indices.size()), 1, 0, 0, 0);

        vkCmdBindDescriptorSets(commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        P1.pipelineLayout, 1, 1, &DS_RightPaddle.descriptorSets[currentImage],
                        0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                    static_cast<uint32_t>(M_Paddle.indices.size()), 1, 0, 0, 0);

        
        for (int i = 0; i < GOAL_SCORE; i++) {
            VkBuffer vertexBuffers4[] = {M_Numbers[i].vertexBuffer};
            VkDeviceSize offsets4[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers4, offsets4);
            vkCmdBindIndexBuffer(commandBuffer, M_Numbers[i].indexBuffer, 0,
                VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                P1.pipelineLayout, 1, 1, &DS_LNumbers[i].descriptorSets[currentImage],
                0, nullptr);
            vkCmdDrawIndexed(commandBuffer,
                static_cast<uint32_t>(M_Numbers[i].indices.size()), 1, 0, 0, 0);

            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                P1.pipelineLayout, 1, 1, &DS_RNumbers[i].descriptorSets[currentImage],
                0, nullptr);
            vkCmdDrawIndexed(commandBuffer,
                static_cast<uint32_t>(M_Numbers[i].indices.size()), 1, 0, 0, 0);
        }


        //Blue Win
        VkBuffer vertexBuffers_BlueWin[] = { M_BlueWin.vertexBuffer };
        VkDeviceSize offsets_BlueWin[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers_BlueWin, offsets_BlueWin);
        vkCmdBindIndexBuffer(commandBuffer, M_BlueWin.indexBuffer, 0,
            VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
         P1.pipelineLayout, 1, 1, &DS_BlueWin.descriptorSets[currentImage],
            0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(M_BlueWin.indices.size()), 1, 0, 0, 0);
        
        //Red Win
        VkBuffer vertexBuffers_RedWin[] = { M_RedWin.vertexBuffer };
        VkDeviceSize offsets_RedWin[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers_RedWin, offsets_RedWin);
        vkCmdBindIndexBuffer(commandBuffer, M_RedWin.indexBuffer, 0,
            VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
         P1.pipelineLayout, 1, 1, &DS_RedWin.descriptorSets[currentImage],
            0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(M_RedWin.indices.size()), 1, 0, 0, 0);
        
        //Reset game
        VkBuffer vertexBuffers_Reset[] = { M_Reset.vertexBuffer };
        VkDeviceSize offsets_Reset[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers_Reset, offsets_Reset);
        vkCmdBindIndexBuffer(commandBuffer, M_Reset.indexBuffer, 0,
            VK_INDEX_TYPE_UINT32);
    
        vkCmdBindDescriptorSets(commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    P1.pipelineLayout, 1, 1, &DS_Reset.descriptorSets[currentImage],
                    0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                    static_cast<uint32_t>(M_Reset.indices.size()), 1, 0, 0, 0);
                
      
    }

    void updateUniformBuffers(uint32_t currentImage) {
        
        globalUniformBufferObject gubo{};
        UniformBufferObject ubo{};
        // updates SkyBox uniforms
        SkyBoxUniformBufferObject subo{};

        void* data;

        float cameraHeight = 1.0f;
        glm::mat4 viewMatrices[NUM_VIEWS + 1] = { 
            glm::lookAt(glm::vec3(0.0f, 1.5f, 2.0f), //Center
                        glm::vec3(0.0f, halfTableHeight, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::lookAt(glm::vec3(-halfTableLength - 0.8f, cameraHeight, 0.0f), //Left player
                        glm::vec3(0.0f, halfTableHeight, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::lookAt(glm::vec3(halfTableLength + 0.8f, cameraHeight, 0.0f), //Right player
                        glm::vec3(0.0f, halfTableHeight, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f))
        };
        
        

        //** spot light **/
        //gubo.lightPos = glm::vec3(0.0f, 4.0f, -4.0f);
        if (skyBox != TOON)
            sel.y = 1.0f;
        else
            sel.y = 0.0f;
            

        gubo.selector=sel;  //Default settings: 1 (noGrey), 1 (Lambert-Phong)
        
        gubo.spotPosition1 = glm::vec3(0.0f, 1.0f, 0.3f);
        gubo.spotPosition2 = glm::vec3(0.0f, 1.0f, -0.3f);

        //gubo.spotDirection = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);
        gubo.spotDirection = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);

        gubo.lightColor = glm::vec3(0.8f, 0.8f, 0.8f);
        if(state == START || state == SETTINGS || state == PAUSE) {
            gubo.ambColor = glm::vec3(1.0f, 1.0f, 1.0f);
            gubo.coneInOutDecayExp = glm::vec4(cos(glm::radians(15.0f)), cos(glm::radians(10.0f)), 0, 2.9f);
        }
        else{
            gubo.ambColor = glm::vec3(0.1f, 0.1f, 0.1f);
            gubo.coneInOutDecayExp = glm::vec4(cos(glm::radians(15.0f)), cos(glm::radians(3.0f)), 2.0f, 3.0f);
        }
        gubo.view = viewMatrices[view];
        gubo.eyePos= viewMatrices[view][3];

        gubo.proj = glm::perspective(glm::radians(45.0f),
            swapChainExtent.width / (float)swapChainExtent.height,
            0.1f, 10.0f);
        gubo.proj[1][1] *= -1;

        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0,
            sizeof(gubo), 0, &data);
        memcpy(data, &gubo, sizeof(gubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

        
        subo.mvpMat = gubo.proj * gubo.view * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f));
        
        for (int i = 0; i < NUM_TEXTURES; i++)
            if (i == skyBox) {
                vkMapMemory(device, DS_SB[i].uniformBuffersMemory[0][currentImage], 0,
                    sizeof(subo), 0, &data);
                memcpy(data, &subo, sizeof(subo));
                vkUnmapMemory(device, DS_SB[i].uniformBuffersMemory[0][currentImage]);
            }                       
        
        
        // For the Table body
        ubo.model = glm::mat4(1.0f);
        for (int i = 0; i < NUM_TEXTURES; i++)
            if (i == skyBox) {
                vkMapMemory(device, DS_Table[i].uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
                memcpy(data, &ubo, sizeof(ubo));
                vkUnmapMemory(device, DS_Table[i].uniformBuffersMemory[0][currentImage]);
            }
                             

        // For the Puck
        const float halfPuckHeight = 0.1f;
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(puck.x, 0.0f, puck.y));

        vkMapMemory(device, DS_Puck.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Puck.uniformBuffersMemory[0][currentImage]);

        // For the lPaddle
        const float halfPaddleHeight = 0.1f;
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(lPaddle.x, 0.0f, lPaddle.y));

        vkMapMemory(device, DS_LeftPaddle.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_LeftPaddle.uniformBuffersMemory[0][currentImage]);

        // For the rPaddle
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(rPaddle.x, 0.0f, rPaddle.y));

        vkMapMemory(device, DS_RightPaddle.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_RightPaddle.uniformBuffersMemory[0][currentImage]);

        
        //Start Screen
        if(state== START || state== SETTINGS || state== PAUSE){
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.48f, 0.56f)) *
            glm::rotate(glm::mat4(1.0), glm::radians(180.0f), glm::vec3(0, 1, 1)) *
            glm::rotate(glm::mat4(1.0), glm::radians(36.0f), glm::vec3(1, 0, 0)) *
            glm::scale(glm::mat4(1.0), glm::vec3(0.08f*swapChainExtent.width / (float)swapChainExtent.height, 0.11f, 0.08f));
            
        } else{
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.48f, 0.56f)) *
                glm::rotate(glm::mat4(1.0), glm::radians(180.0f), glm::vec3(0, 1, 1)) *
                glm::rotate(glm::mat4(1.0), glm::radians(36.0f), glm::vec3(1, 0, 0)) *
                glm::scale(glm::mat4(1.0), glm::vec3(0.11f, 0.11f, 0.08f));
        }
        
        switch (state) {
            case START:
                vkMapMemory(device, DS_StartScreen.uniformBuffersMemory[0][currentImage], 0,
                            sizeof(ubo), 0, &data);
                memcpy(data, &ubo, sizeof(ubo));
                vkUnmapMemory(device, DS_StartScreen.uniformBuffersMemory[0][currentImage]);
                break;
            case SETTINGS:                
                vkMapMemory(device, DS_Settings.uniformBuffersMemory[0][currentImage], 0,
                            sizeof(ubo), 0, &data);
                memcpy(data, &ubo, sizeof(ubo));
                vkUnmapMemory(device, DS_Settings.uniformBuffersMemory[0][currentImage]);
                break;
            case PAUSE:                
                vkMapMemory(device, DS_Pause.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
                memcpy(data, &ubo, sizeof(ubo));
                vkUnmapMemory(device, DS_Pause.uniformBuffersMemory[0][currentImage]);
                break;
            case RESUME:                
                vkMapMemory(device, DS_Pause.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
                memcpy(data, &ubo, sizeof(ubo));
                vkUnmapMemory(device, DS_Pause.uniformBuffersMemory[0][currentImage]);
                break;
        }                  

        const float scoreHeight = 0.3f;
        for (int i = 0; i < GOAL_SCORE; i++) {
            float visibleZ = 100.0f;
            //For the lScore
            if (i == leftPlayerScore)
                visibleZ = lScore.y;
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(lScore.x, scoreHeight, visibleZ)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

            vkMapMemory(device, DS_LNumbers[i].uniformBuffersMemory[0][currentImage], 0,
                sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, DS_LNumbers[i].uniformBuffersMemory[0][currentImage]);
            visibleZ = 100.0f;

            // For the rScore
            if (i == rightPlayerScore)
                visibleZ = rScore.y;
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(rScore.x, scoreHeight, visibleZ)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

            vkMapMemory(device, DS_RNumbers[i].uniformBuffersMemory[0][currentImage], 0,
                sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, DS_RNumbers[i].uniformBuffersMemory[0][currentImage]);
        }
        
        //Win
        if (state == VICTORY) {
            winTextAngle += dt;
            switch (winner) {
                case LEFT:
                    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, 0.0f))*
                        glm::scale(glm::mat4(1.0),glm::vec3(0.15f)) * 
                        glm::rotate(glm::mat4(1.0), glm::radians(270.0f), glm::vec3(0,1,0));
                    ubo.model = glm::rotate(ubo.model, glm::radians(30.0f) * cos(winTextAngle/2), glm::vec3(0.0f, 1.0f, 0.0f));
                    //TODO What happens if we multiply instead of rotating the translated matrix? Insert inside rotate above
                                
                    vkMapMemory(device, DS_BlueWin.uniformBuffersMemory[0][currentImage], 0,
                                        sizeof(ubo), 0, &data);
                    memcpy(data, &ubo, sizeof(ubo));
                    vkUnmapMemory(device, DS_BlueWin.uniformBuffersMemory[0][currentImage]);
                    
                    //Reset
                    
                    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 0.2f, 0.0f)) *
                        glm::scale(glm::mat4(1.0),glm::vec3(0.08f)) * 
                        glm::rotate(glm::mat4(1.0), glm::radians(270.0f), glm::vec3(0,1,0)) *
                        glm::rotate(glm::mat4(1.0), glm::radians(-30.0f), glm::vec3(1,0,0));
                    
                    
                    vkMapMemory(device, DS_Reset.uniformBuffersMemory[0][currentImage], 0,
                                        sizeof(ubo), 0, &data);
                    memcpy(data, &ubo, sizeof(ubo));
                    vkUnmapMemory(device, DS_Reset.uniformBuffersMemory[0][currentImage]);
                
                    break;
                    
                case RIGHT:
                    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, 0.0f))*
                        glm::scale(glm::mat4(1.0),glm::vec3(0.15f)) * 
                        glm::rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(0, 1, 0));
                    ubo.model = glm::rotate(ubo.model, glm::radians(30.0f) * cos(winTextAngle/2), glm::vec3(0.0f, 1.0f, 0.0f));
                        
                    vkMapMemory(device, DS_RedWin.uniformBuffersMemory[0][currentImage], 0,
                                        sizeof(ubo), 0, &data);
                    memcpy(data, &ubo, sizeof(ubo));
                    vkUnmapMemory(device, DS_RedWin.uniformBuffersMemory[0][currentImage]);
                    
                    //Reset
                    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.2f, 0.0f))*
                        glm::scale(glm::mat4(1.0),glm::vec3(0.08f)) * 
                        glm::rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(0,1,0)) *
                        glm::rotate(glm::mat4(1.0), glm::radians(-30.0f), glm::vec3(1,0,0));
      
                    vkMapMemory(device, DS_Reset.uniformBuffersMemory[0][currentImage], 0,
                                        sizeof(ubo), 0, &data);
                    memcpy(data, &ubo, sizeof(ubo));
                    vkUnmapMemory(device, DS_Reset.uniformBuffersMemory[0][currentImage]);
                    break;
            }
    
        }
        else {
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.4f, 0.2f)) *
                glm::scale(glm::mat4(1.0),glm::vec3(0.2f));
             
            //WinTexts
            vkMapMemory(device, DS_BlueWin.uniformBuffersMemory[0][currentImage], 0,
                                sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, DS_BlueWin.uniformBuffersMemory[0][currentImage]);                                
                                
            vkMapMemory(device, DS_RedWin.uniformBuffersMemory[0][currentImage], 0,
                                        sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, DS_RedWin.uniformBuffersMemory[0][currentImage]);
            
            //ResetText
            vkMapMemory(device, DS_Reset.uniformBuffersMemory[0][currentImage], 0,
                                sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, DS_Reset.uniformBuffersMemory[0][currentImage]);
        }
        
    }

    void checkSkyBoxChanges(){
        if (glfwGetKey(window, GLFW_KEY_C) && isDebounced()){
            skyBox = static_cast<skyBoxes>((skyBox + 1) % NUM_TEXTURES);
            commandBufferUpdate=true;
        }
    }
    
    void checkChangeDifficulty() {
        if (glfwGetKey(window, GLFW_KEY_F) && difficulty != EASY) {
            puck.vx /= 1.4;
            puck.vy /= 1.4;
            puckVelocity /= 1.4;
            if (difficulty == NORMAL)
                difficulty = EASY;
            else
                difficulty = NORMAL;
        }

        if (glfwGetKey(window, GLFW_KEY_R) && difficulty != HARD) {
            puck.vx *= 1.4;
            puck.vy *= 1.4;
            puckVelocity *= 1.4;
            if (difficulty == NORMAL)
                difficulty = HARD;
            else
                difficulty = NORMAL;
        }
    }

    void checkPaddlesMovement() {
        switch (view) {
            case ABOVE:
                if (glfwGetKey(window, GLFW_KEY_A))
                    lPaddle.vx -= paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_D))
                    lPaddle.vx += paddleVelocity;
          
                if (glfwGetKey(window, GLFW_KEY_W))
                    lPaddle.vy -= paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_S))
                    lPaddle.vy += paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_LEFT))
                    rPaddle.vx -= paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_RIGHT))
                    rPaddle.vx += paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_UP))  //y-axis is reversed to match z-axis later during matrix creation
                    rPaddle.vy -= paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_DOWN))
                    rPaddle.vy += paddleVelocity;
                break;
            
            case LEFTPLAYER:
                if (glfwGetKey(window, GLFW_KEY_S))
                    lPaddle.vx -= paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_W))
                    lPaddle.vx += paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_A))
                    lPaddle.vy -= paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_D))
                    lPaddle.vy += paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_DOWN))
                    rPaddle.vx -= paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_UP))
                    rPaddle.vx += paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_LEFT))   //y-axis is reversed to match z-axis later during matrix creation
                    rPaddle.vy -= paddleVelocity;
                
                if (glfwGetKey(window, GLFW_KEY_RIGHT))
                    rPaddle.vy += paddleVelocity;
                break;
            
            case RIGHTPLAYER:
                if (glfwGetKey(window, GLFW_KEY_W))
                    lPaddle.vx -= paddleVelocity;

                if (glfwGetKey(window, GLFW_KEY_S))
                    lPaddle.vx += paddleVelocity;

                if (glfwGetKey(window, GLFW_KEY_D))
                    lPaddle.vy -= paddleVelocity;

                if (glfwGetKey(window, GLFW_KEY_A))
                    lPaddle.vy += paddleVelocity;

                if (glfwGetKey(window, GLFW_KEY_UP))
                    rPaddle.vx -= paddleVelocity;

                if (glfwGetKey(window, GLFW_KEY_DOWN))
                    rPaddle.vx += paddleVelocity;

                if (glfwGetKey(window, GLFW_KEY_RIGHT))   //y-axis is reversed to match z-axis later during matrix creation
                    rPaddle.vy -= paddleVelocity;

                if (glfwGetKey(window, GLFW_KEY_LEFT))
                    rPaddle.vy += paddleVelocity;
                break;
        }
    }

    //TODO Could avoid repeating twice the same piece of code
    void updatePaddlesPosition() {
        float tempPosition;
        bool update = false;

        tempPosition = lPaddle.x + lPaddle.vx * dt;
        if (tempPosition + paddleRadius <= -scoreAreaX) {
            if (abs(lPaddle.y) > cornerCircleRadius) {
                float relativeX = abs(tempPosition) - abs(cornerCircleCenter.x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
                float relativeY = abs(lPaddle.y) - cornerCircleRadius;
                float maxRelativeX = sqrt(pow((cornerCircleRadius - paddleRadius), 2) - pow(relativeY, 2));
                if (relativeX <= maxRelativeX)
                    update = true;
            }
            else
                if (tempPosition - paddleRadius >= -halfTableLength)
                    update = true;
        }
        if (update) {
            lPaddle.x = tempPosition;
            update = false;
        }

        tempPosition = lPaddle.y + lPaddle.vy * dt;
        if (abs(tempPosition) + paddleRadius <= halfTableWidth) {
            if (abs(lPaddle.x) > abs(cornerCircleCenter.x)) {
                float relativeX = abs(lPaddle.x) - abs(cornerCircleCenter.x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
                float relativeY = abs(tempPosition) - cornerCircleRadius;
                float maxRelativeY = sqrt(pow((cornerCircleRadius - paddleRadius), 2) - pow(relativeX, 2));
                if (relativeY <= maxRelativeY)
                    update = true;
            }
            else
                update = true;
        }
        if (update) {
            lPaddle.y = tempPosition;
            update = false;
        }


        tempPosition = rPaddle.x + rPaddle.vx * dt;
        if (tempPosition - paddleRadius >= scoreAreaX) {
            if (abs(rPaddle.y) > cornerCircleRadius) {//RT, RB
                float relativeX = abs(tempPosition) - abs(cornerCircleCenter.x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
                float relativeY = abs(rPaddle.y) - cornerCircleRadius;
                float maxRelativeX = sqrt(pow((cornerCircleRadius - paddleRadius), 2) - pow(relativeY, 2));
                if (relativeX <= maxRelativeX)
                    update = true;
            }
            else
                if (tempPosition + paddleRadius <= halfTableLength)
                    update = true;
        }
        if (update) {
            rPaddle.x = tempPosition;
            update = false;
        }

        tempPosition = rPaddle.y + rPaddle.vy * dt;
        if (abs(tempPosition) + paddleRadius <= halfTableWidth) {
            if (abs(rPaddle.x) > abs(cornerCircleCenter.x)) {
                float relativeX = abs(rPaddle.x) - abs(cornerCircleCenter.x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
                float relativeY = abs(tempPosition) - cornerCircleRadius;
                float maxRelativeY = sqrt(pow((cornerCircleRadius - paddleRadius), 2) - pow(relativeX, 2));
                if (relativeY <= maxRelativeY)
                    update = true;
            }
            else
                update = true;
        }
        if (update)
            rPaddle.y = tempPosition;
    }

    glm::vec2 calculateBounceDirection() {
        glm::vec2 normalVector = glm::vec2(0.0f, 0.0f);
        //Corner collision detection code
        
        //Used for puck-paddle collision check
        const float puckPaddleMinDistance = paddleRadius + puckRadius;
        float puckLeftPaddleDistanceX = puck.x - lPaddle.x;
        float puckLeftPaddleDistanceY = puck.y - lPaddle.y;

        float puckRightPaddleDistanceX = puck.x - rPaddle.x;
        float puckRightPaddleDistanceY = puck.y - rPaddle.y;

        //Used for borders-puck collision check
        const float relativeRadius = cornerCircleRadius - puckRadius;
        float relativeX = abs(puck.x) - abs(cornerCircleCenter.x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
        float relativeY = abs(puck.y) - cornerCircleRadius;

        const float halfGoalWidth = halfTableWidth - cornerCircleRadius;
        float edgePuckDistanceX = halfTableLength - abs(puck.x);
        float edgePuckDistanceY = halfGoalWidth - abs(puck.y);


        if (relativeX > 0 &&
            relativeY > 0 &&
            (pow(relativeX, 2) + pow(relativeY, 2) >= pow(relativeRadius, 2))) {
            if (completelyBounced) {
                normalVector = glm::normalize(glm::vec2(
                    relativeX * ((puck.x < 0) ? 1.0f : -1.0f), //If this parentheses are removed after *, it breaks down awfully. What does it get assigned exactly x) ?
                    relativeY * ((puck.y < 0) ? 1.0f : -1.0f)
                ));
                completelyBounced = false;
            }
        }
        //Each case has to be handled separately otherwise it's not possible to determine which normal vector has to be used
        else if (puck.y >= halfTableWidth - puckRadius) { //UPPER SIDE
            if (completelyBounced) {
                normalVector = glm::vec2(0.0f, -1.0f);
                completelyBounced = false;
            }
        }
        else if (puck.y <= -halfTableWidth + puckRadius) {
            if (completelyBounced) {
                normalVector = glm::vec2(0.0f, 1.0f); //LOWER SIDE
                completelyBounced = false;
            }
            
        else if (pow(edgePuckDistanceX, 2) + pow(edgePuckDistanceY, 2) <= pow(puckRadius, 2))
                if (completelyBounced) { 
                    normalVector = glm::normalize(glm::vec2( //PUCK-EDGES
                        edgePuckDistanceX, 
                        edgePuckDistanceY
                    ));
                    completelyBounced = false;
                }   
        }
        else if (pow(puckLeftPaddleDistanceX, 2) + pow(puckLeftPaddleDistanceY, 2) <= pow(puckPaddleMinDistance, 2)) {
            if (completelyBounced) {
                normalVector = glm::normalize(glm::vec2(
                    puckLeftPaddleDistanceX, //correct sign included in the distances calculated above
                    puckLeftPaddleDistanceY
                ));
                completelyBounced = false;
            }
        }
        else if (pow(puckRightPaddleDistanceX, 2) + pow(puckRightPaddleDistanceY, 2) <= pow(puckPaddleMinDistance, 2)) {
            if (completelyBounced) {
                normalVector = glm::normalize(glm::vec2(
                    puckRightPaddleDistanceX, //correct sign included in the distances calculated above
                    puckRightPaddleDistanceY
                ));
                completelyBounced = false;
            }
        } else
            completelyBounced = true;
        //IMPORTANT ASSUMPTION: paddle velocity is always lower than puck velocity. Otherwise, if the paddle moved in the direction of the puck velocity,
        //compenetration would occur, debouncing time could expire and a second collision could be triggered
        
        return normalVector;
    }

    void launchPuck() {
        float randomInitialAngleCos = cos(initialPuckAngle) + (1 - cos(initialPuckAngle)) * (rand() % 100) / 100.0f;
        float randomInitialAngleSin = sqrt(1 - pow(randomInitialAngleCos, 2));
        puck.vx = puckVelocity * randomInitialAngleCos * ((rand() % 2 == 0) ? -1 : 1);
        puck.vy = puckVelocity * randomInitialAngleSin * ((rand() % 2 == 0) ? -1 : 1);
    }

    //Calculate new vx, vy using the normal vector as done in the Phong model:
    //Normalize v, flip it (-1) and extract the modulus -> If defined separately at 266 (M * component), modulus is given  --> UPDATE: Normalization not required if normal is provided normalized (or is normalized below)
    //Dot product with normal, 2*result - initial normalized vector, times M
    void updatePuckVelocity(glm::vec2 &normalVector) {
        
        glm::vec2 flippedPuckVel = glm::vec2(-puck.vx, -puck.vy);//Flip puck.v, as in Phong
        float velProjectionModulus = dot(flippedPuckVel, normalVector); 

        glm::vec2 finalVelocity; 
        if (velProjectionModulus >= 0) { //Collision occurred while paddle was still - no external forces applied
            glm::vec2 velProjection = glm::vec2(normalVector * velProjectionModulus);

            finalVelocity = glm::vec2(
                2.0f * velProjection - flippedPuckVel
            );
        } else 
            //For the sake of simplicity, let's assume that, when an external force is applied, 
            //it's strong enough to force the puck along the paddle radius
            finalVelocity = glm::vec2(
                normalVector * puckVelocity
            );
       

        puck.vx = finalVelocity.x;
        puck.vy = finalVelocity.y;
    }

    void checkScoreOccurred() {
        if (puck.x <= -halfTableLength) {
            rightPlayerScore++;
            if (rightPlayerScore == GOAL_SCORE){
                winner = RIGHT;
                endGame();
            } else
                resetBoard();

        } else if (puck.x >= halfTableLength) {
            leftPlayerScore++;
            if (leftPlayerScore == GOAL_SCORE){
                winner = LEFT;
                endGame();
            } else
                resetBoard();
        }
    }

    bool isDebounced() {
        float debounceInterval = std::chrono::duration<float>(currentTime - debounceTime).count();
        if (debounceInterval > DEBOUNCE_THRESHOLD) {
            debounceTime = currentTime;
            return true;
        }
        return false;
    }

    void checkChangeView() {
        if (glfwGetKey(window, GLFW_KEY_V) && isDebounced()) 
            view = static_cast<views>((view+1) % NUM_VIEWS);
          
    }

    void resetBoard() {
        puck.x = 0.0f;
        puck.y = 0.0f;

        puck.vx = 0.0f;
        puck.vy = 0.0f;

        lPaddle.x = -halfTableLength + paddleRadius;
        lPaddle.y = 0.0f; //What would happen here if I assigned a whole new object?
        rPaddle.x = halfTableLength - paddleRadius;
        rPaddle.y = 0.0f;

        state = RESET;
    }

    void endGame() {
        state = VICTORY;
        oldView = view;                
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void gameLoop(uint32_t currentImage) {
        currentTime = std::chrono::system_clock::now();
        dt = std::chrono::duration<float>(currentTime - lastTime).count() * TIME_SPEED;

        /*if (glfwGetKey(window, GLFW_KEY_ESCAPE))
            glfwSetWindowMonitor(window, NULL, 100, 20, windowWidth, windowHeight, 120);
        //TODO: if we want to add fullscreen -> windowed, we have to recreate the swapChain.
        //See https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation
        */
        checkSkyBoxChanges();
        switch (state) {
            case START:            
                if (glfwGetKey(window, GLFW_KEY_P) && isDebounced()) {
                    state = SETTINGS;
                    commandBufferUpdate = true;
                }
                break;
        
            case SETTINGS:
                if (glfwGetKey(window, GLFW_KEY_P) && isDebounced()) {
                    resetBoard();
                    view = ABOVE;
                    commandBufferUpdate = true;
                }
                break;
    
            case RESET:
                checkChangeDifficulty();
                if (glfwGetKey(window, GLFW_KEY_SPACE)) {
                    launchPuck();
                    state = PLAYING;
                }
                checkChangeView();
                break;
        
            case PLAYING:
            {
                checkChangeDifficulty();

                lPaddle.vx = 0.0f;
                lPaddle.vy = 0.0f;
                
                rPaddle.vx = 0.0f;
                rPaddle.vy = 0.0f;

                checkPaddlesMovement();

                updatePaddlesPosition();

                glm::vec2 normalVector = calculateBounceDirection();

                if (glm::length(normalVector) != 0.0f) { //Collision detected
                    updatePuckVelocity(normalVector);
                    completelyBounced = false;   
                }

                puck.x += puck.vx * dt;
                puck.y += puck.vy * dt;

                checkScoreOccurred();

                if (glfwGetKey(window, GLFW_KEY_T)) { //Test setting, set to custom position
                    puck.x = halfTableLength - puckRadius - 0.2f;
                    puck.y = halfTableWidth - puckRadius - 0.2f;
                    launchPuck();
                }
                
                checkChangeView();
                
                if (glfwGetKey(window, GLFW_KEY_P) && isDebounced()) {                    
                    puck.vx=0;
                    puck.vy=0;

                    state=PAUSE;
                    oldView=view;
                    view=ABOVE;

                    sel.x = 0.0f; //Screen becomes grey
                    commandBufferUpdate=true;
                }                
                break;
            }
            case VICTORY: 

                if (winner == RIGHT) 
                    view = RIGHTPLAYER;
                else 
                    view = LEFTPLAYER;
                
                rightPlayerScore = -1;
                leftPlayerScore = -1;
                
                if (glfwGetKey(window, GLFW_KEY_R) && isDebounced()) {
                    rightPlayerScore = 0;
                    leftPlayerScore = 0;
                    view = oldView;
                    resetBoard();
                }
                break;
                
            case PAUSE:
                
                if (glfwGetKey(window, GLFW_KEY_P) && isDebounced()) {
                    state = RESUME;
                    view = oldView;                                     
                }
                
                if (glfwGetKey(window, GLFW_KEY_L) && isDebounced() ) {
                    rightPlayerScore = 0;
                    leftPlayerScore = 0;
                    view = oldView;
                    resetBoard();

                    sel.x = 1.0f;
                    commandBufferUpdate=true;                    
                }
                break;
                
            case RESUME:
                float intervalAfterResuming = std::chrono::duration<float>(currentTime - debounceTime).count();
                if (intervalAfterResuming > REACTION_TIME){
                    sel.x = 1.0f;
                    state=PLAYING;
                    launchPuck();
                }    
                break;
        }

        updateUniformBuffers(currentImage);

        lastTime = currentTime;
    }
};

// This is the main: probably you do not need to touch this!
int main() {
    MyProject app;

    srand(time(NULL));

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

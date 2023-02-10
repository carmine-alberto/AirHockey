// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

#define NUM_VIEWS 3
#define NUM_CORNERS 4
#define GOAL_SCORE 7

//const std::string SKYBOX_TEXTURE[6] = { "textures/skybox/negx.jpg", "textures/skybox/negy.jpg", "textures/skybox/negz.jpg", "textures/skybox/posx.jpg", "textures/skybox/posy.jpg", "textures/skybox/posz.jpg" }; TODO Fix or remove
  

// The uniform buffer object used in this example
struct globalUniformBufferObject {
    alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec3 lightColor;
        alignas(16) glm::vec3 lightPos;
        alignas(16) glm::vec3 ambColor;
        alignas(16) glm::vec4 coneInOutDecayExp;
        alignas(16) glm::vec3 spotPosition1;  //TODO No way we can pass an array?
        alignas(16) glm::vec3 spotPosition2;
        alignas(16) glm::vec3 spotPosition3;
        alignas(16) glm::vec3 spotPosition4;
        alignas(16) glm::vec3 spotPosition5;
        alignas(16) glm::vec3 spotPosition6;
        alignas(16) glm::vec3 spotPosition7;
        alignas(16) glm::vec3 spotPosition8;
        alignas(16) glm::vec3 spotDirection1;
        alignas(16) glm::vec3 spotDirection2;
        alignas(16) glm::vec3 spotDirection3;
        alignas(16) glm::vec3 spotDirection4;
        alignas(16) glm::vec3 spotDirection5;
        alignas(16) glm::vec3 spotDirection6;
        alignas(16) glm::vec3 spotDirection7;
        alignas(16) glm::vec3 spotDirection8;
        alignas(16) glm::vec3 eyePos;
    
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
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

    //SkyBox
    Model M_SB;
    Texture T_SB;
    DescriptorSet DS_SB;
    
     
    // Models, textures and Descriptors (values assigned to the uniforms)
    Model M_Table;
    Texture T_Table;
    DescriptorSet DS_Table;    // instance DSLobj
    
    //Puck
    Model M_Puck;
    Texture T_Puck;
    DescriptorSet DS_Puck;    // instance DSLobj

    //Paddle
    Model M_Paddle;
    Texture T_LeftPaddle;
    Texture T_RightPaddle;
    DescriptorSet DS_LeftPaddle;    // instance DSLobj
    DescriptorSet DS_RightPaddle;    // instance DSLobj

    //Start Screen
    Model M_StartScreen;
    Texture T_StartScreen;
    DescriptorSet DS_StartScreen;

    //Points
    Model M_Numbers[GOAL_SCORE];
    Texture T_LNumbers;
    Texture T_RNumbers;
    DescriptorSet DS_LNumbers[GOAL_SCORE];
    DescriptorSet DS_RNumbers[GOAL_SCORE];
    
    DescriptorSet DS_global;

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
    float puckVelocity = 0.5f;
    float initialPuckAngle = glm::radians(60.0f);
    Point puck = { 0.0f, 0.0f, 0.0f, 0.0f };

    float paddleRadius = 0.07;
    float paddleVelocity = 0.2f;
    
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
        RESET,
        PLAYING,
        VICTORY
    } state = START;

    enum views {
        STARTSCREEN,
        ABOVE,
        LEFTPLAYER,
        RIGHTPLAYER
    } view = STARTSCREEN;

    float dt = 0.01f;
    std::chrono::time_point<std::chrono::system_clock> lastTime = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> debounceTime = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> currentTime;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 1800;
		windowHeight = 1000;
		windowTitle = "Air Hockey";
		initialBackgroundColor = {0.0f, 0.5f, 0.0f, 1.0f};
		
		// Descriptor pool sizes
        //TODO Check the number is tight
		uniformBlocksInPool = 24;
		texturesInPool = 10;
		setsInPool = 24;
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLobj.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the time of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				  });

        DSLglobal.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                  });
            

        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on..
        P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSLglobal, &DSLobj});
        
        // Models, textures and Descriptors (values assigned to the uniforms)
        M_Table.init(this, "models/table.obj");
        T_Table.init(this, "textures/airHockey3.png");
        DS_Table.init(this, &DSLobj, {
        // the second parameter, is a pointer to the Uniform Set Layout of this set
        // the last parameter is an array, with one element per binding of the set.
        // first  elmenet : the binding number
        // second element : UNIFORM or TEXTURE (an enum) depending on the type
        // third  element : only for UNIFORMs, the size of the corresponding C++ object
        // fourth element : only for TEXTUREs, the pointer to the corresponding texture object
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &T_Table}
                });
    
        //Puck
        M_Puck.init(this, "models/disk.obj");
        T_Puck.init(this, "textures/disk.png");
        DS_Puck.init(this, &DSLobj, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &T_Puck}
                });

        //Paddle
        M_Paddle.init(this, "models/paddle.obj"); //TODO Rename
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

        //Sky Box
        M_SB.init(this, "models/SkyBoxCube.obj");
        T_SB.init(this, "textures/blue/bkg1_top.png");
        DS_SB.init(this, &DSLobj, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &T_SB}
                });
        
        
        //Start Screen
        M_StartScreen.init(this, "models/ground.obj");
        T_StartScreen.init(this, "textures/StartScreen.png");
        DS_StartScreen.init(this, &DSLobj, {
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
            {1, TEXTURE, 0, &T_StartScreen}
        });

        //Numbers
        /*T_LNumbers.init(this, "textures/leftNumbers.png");
        T_RNumbers.init(this, "textures/rightNumbers.png");
        TODO Do we want to give numbers a different texture?*/
        for (int i = 0; i < GOAL_SCORE; i++) {
            M_Numbers[i].init(this, "models/" + std::to_string(i) + ".obj");

            DS_LNumbers[i].init(this, &DSLobj, {
                        {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                        {1, TEXTURE, 0, &T_LeftPaddle}
                });
            DS_RNumbers[i].init(this, &DSLobj, {
                        {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                        {1, TEXTURE, 0, &T_RightPaddle}
                });
        }

        DS_global.init(this, &DSLglobal, {
                    {0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}
                });
        
    }

    // Here you destroy all the objects you created!
    //TODO Check everything is destroyed properly
    void localCleanup() {
        //Table
        DS_Table.cleanup();
        T_Table.cleanup();
        M_Table.cleanup();
        
        //SkyBox
        DS_SB.cleanup();
        T_SB.cleanup();
        M_SB.cleanup();
         
        
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
        T_StartScreen.cleanup();
        M_StartScreen.cleanup();
        
        DS_global.cleanup();

        P1.cleanup();
        
        DSLglobal.cleanup();
        DSLobj.cleanup();
        
    }
    
    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
                
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

        // property .pipelineLayout of a pipeline contains its layout.
        // property .descriptorSets of a descriptor set contains its elements.
        vkCmdBindDescriptorSets(commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        P1.pipelineLayout, 1, 1, &DS_Table.descriptorSets[currentImage],
                        0, nullptr);
                        
        // property .indices.size() of models, contains the number of triangles * 3 of the mesh.
        vkCmdDrawIndexed(commandBuffer,
                    static_cast<uint32_t>(M_Table.indices.size()), 1, 0, 0, 0);

        
        VkBuffer vertexBuffers_SC[] = {M_StartScreen.vertexBuffer};
        VkDeviceSize offsets_SC[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers_SC, offsets_SC);
        vkCmdBindIndexBuffer(commandBuffer, M_StartScreen.indexBuffer, 0,
                                VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        P1.pipelineLayout, 1, 1, &DS_StartScreen.descriptorSets[currentImage],
                        0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                    static_cast<uint32_t>(M_StartScreen.indices.size()), 1, 0, 0, 0);
        
        
        
        //SkyBox buffer initialization
        VkBuffer vertexBuffers_SkyBox[] = { M_SB.vertexBuffer };
        VkDeviceSize offsets_SkyBox[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers_SkyBox, offsets_SkyBox);
        vkCmdBindIndexBuffer(commandBuffer, M_SB.indexBuffer, 0,
            VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
         P1.pipelineLayout, 1, 1, &DS_SB.descriptorSets[currentImage],
            0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(M_SB.indices.size()), 1, 0, 0, 0);
        
        

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


      

    }

    void updateGPUData(uint32_t currentImage) {
        globalUniformBufferObject gubo{};
        UniformBufferObject ubo{};

        void* data;

        float cameraHeight = 1.0f;
        glm::mat4 viewMatrices[NUM_VIEWS + 1] = { //TODO Refactor into different states
            glm::lookAt(glm::vec3(0.0f, 10.0f, 0.000000001f), //Start Screen
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 2.0f, 1.0f), //Center
                        glm::vec3(0.0f, halfTableHeight, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::lookAt(glm::vec3(-halfTableLength - 0.8f, cameraHeight, 0.0f), //Left player
                        glm::vec3(0.0f, halfTableHeight, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::lookAt(glm::vec3(halfTableLength + 0.8f, cameraHeight, 0.0f), //Right player
                        glm::vec3(0.0f, halfTableHeight, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f)),
        };

        //** spot light **/
        //gubo.lightPos = glm::vec3(0.0f, 4.0f, -4.0f);
        gubo.spotPosition1 = glm::vec3(-0.7f, 1.0f, 0.2f);
        gubo.spotPosition2 = glm::vec3(-0.7f, 1.0f, -0.2f);
        gubo.spotPosition3 = glm::vec3(0.7f, 1.0f, 0.2f);
        gubo.spotPosition4 = glm::vec3(0.7f, 1.0f, -0.2f);
        gubo.spotPosition5 = glm::vec3(-0.2f, 1.0f, 0.2f);
        gubo.spotPosition6 = glm::vec3(-0.2f, 1.0f, -0.2f);
        gubo.spotPosition7 = glm::vec3(0.2f, 1.0f, 0.2f);
        gubo.spotPosition8 = glm::vec3(0.2f, 1.0f, -0.2f);

        gubo.spotDirection1 = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);
        gubo.spotDirection2 = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);
        gubo.spotDirection3 = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);
        gubo.spotDirection4 = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);
        gubo.spotDirection5 = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);
        gubo.spotDirection6 = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);
        gubo.spotDirection7 = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);
        gubo.spotDirection8 = glm::vec3(cos(glm::radians(90.0f)), sin(glm::radians(90.0f)), 0.0f);

        gubo.lightColor = glm::vec3(0.6f, 0.6f, 0.6f);
        gubo.ambColor = glm::vec3(0.2f, 0.2f, 0.2f);
        gubo.coneInOutDecayExp = glm::vec4(0.92f, 0.99f, 0.8f, 2.0f);

        gubo.view = viewMatrices[view];

        gubo.proj = glm::perspective(glm::radians(45.0f),
            swapChainExtent.width / (float)swapChainExtent.height,
            0.1f, 10.0f);
        gubo.proj[1][1] *= -1;

        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0,
            sizeof(gubo), 0, &data);
        memcpy(data, &gubo, sizeof(gubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);


        // For the Table body
        ubo.model = glm::mat4(1.0f);
        vkMapMemory(device, DS_Table.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Table.uniformBuffersMemory[0][currentImage]);

        // For the Puck
        const float halfPuckHeight = 0.1f; //TODO Sync with model
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

        //SkyBox
        ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(5.5f)) *
            glm::rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(0, 1, 0));

        vkMapMemory(device, DS_SB.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_SB.uniformBuffersMemory[0][currentImage]);


        //Start Screen
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 0.0f)) *
            glm::rotate(glm::mat4(1.0), glm::radians(180.0f), glm::vec3(0, 1, 0)) *
            glm::scale(glm::mat4(1.0), glm::vec3(0.4f, 0.5f, 0.27f));

        vkMapMemory(device, DS_StartScreen.uniformBuffersMemory[0][currentImage], 0,
            sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_StartScreen.uniformBuffersMemory[0][currentImage]);

        const float scoreHeight = 0.2f;
        for (int i = 0; i < GOAL_SCORE; i++) {
            float visibleZ = 100.0f;
            //For the lScore
            if (i == leftPlayerScore)
                visibleZ = lScore.y;
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(lScore.x, scoreHeight, visibleZ)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));

            vkMapMemory(device, DS_LNumbers[i].uniformBuffersMemory[0][currentImage], 0,
                sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, DS_LNumbers[i].uniformBuffersMemory[0][currentImage]);
            visibleZ = 100.0f;

            // For the rScore
            if (i == rightPlayerScore)
                visibleZ = rScore.y;
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(rScore.x, scoreHeight, visibleZ)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));

            vkMapMemory(device, DS_RNumbers[i].uniformBuffersMemory[0][currentImage], 0,
                sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, DS_RNumbers[i].uniformBuffersMemory[0][currentImage]);
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
        std::chrono::duration<float> debounceInterval = (currentTime - debounceTime);
        //Corner collision detection code

        if (debounceInterval.count() > 0.05f) {
            float relativeX = abs(puck.x) - abs(cornerCircleCenter.x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
            float relativeY = abs(puck.y) - cornerCircleRadius;
            const float relativeRadius = cornerCircleRadius - puckRadius;

            if (relativeX > 0 &&
                relativeY > 0 &&
                (pow(relativeX, 2) + pow(relativeY, 2) >= pow(relativeRadius, 2)))
                normalVector = glm::normalize(glm::vec2(
                    relativeX * ((puck.x < 0) ? 1.0f : -1.0f), //If this parentheses are removed after *, it breaks down awfully. What does it get assigned exactly x) ?
                    relativeY * ((puck.y < 0) ? 1.0f : -1.0f)
                ));
                
            //Each case has to be handled separately otherwise it's not possible to determine which normal vector has to be used
            else if (puck.y >= halfTableWidth - puckRadius)  //UPPER SIDE
                normalVector = glm::vec2(0.0f, 1.0f);
            else if (puck.y <= -halfTableWidth + puckRadius)
                normalVector = glm::vec2(0.0f, -1.0f); //LOWER SIDE
            /* TODO Handle these cases - hit on edges
            else if (puck.x <= -halfTableLength + puckRadius)
                normalVector = glm::vec2(1.0f, 0.0f); //LEFT SIDE
            else if (puck.x >= halfTableLength - puckRadius)
                normalVector = glm::vec2(-1.0f, 0.0f); //RIGHT SIDE
                */
            else {
                const float puckPaddleMinDistance = paddleRadius + puckRadius;
                float puckLeftPaddleDistanceX = puck.x - lPaddle.x;
                float puckLeftPaddleDistanceY = puck.y - lPaddle.y;

                float puckRightPaddleDistanceX = puck.x - rPaddle.x;
                float puckRightPaddleDistanceY = puck.y - rPaddle.y;

                if (pow(puckLeftPaddleDistanceX, 2) + pow(puckLeftPaddleDistanceY, 2) <= pow(puckPaddleMinDistance, 2))
                    normalVector = glm::normalize(glm::vec2(
                        puckLeftPaddleDistanceX, //correct sign included in the distances calculated above
                        puckLeftPaddleDistanceY
                    ));
                else if (pow(puckRightPaddleDistanceX, 2) + pow(puckRightPaddleDistanceY, 2) <= pow(puckPaddleMinDistance, 2))
                    normalVector = glm::normalize(glm::vec2(
                        puckRightPaddleDistanceX, //correct sign included in the distances calculated above
                        puckRightPaddleDistanceY
                    ));
            }
            //IMPORTANT ASSUMPTION: paddle velocity is always lower than puck velocity. Otherwise, if the paddle moved in the direction of the puck velocity, 
            //compenetration would occur, debouncing time could expire and a second collision could be triggered
        }
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
        glm::vec2 flippedPuckVel = glm::vec2(-puck.vx, -puck.vy);
        float velProjectionModulus = dot(flippedPuckVel, normalVector); //Flip puck.v, as in Phong
        glm::vec2 velProjection = glm::vec2(normalVector * velProjectionModulus);

        glm::vec2 finalVelocity = glm::vec2(
            2.0f * velProjection - flippedPuckVel
        );

        puck.vx = finalVelocity.x;
        puck.vy = finalVelocity.y;
    }

    void checkScoreOccurred() {
        if (puck.x <= -halfTableLength) {
            rightPlayerScore++;
            if (rightPlayerScore == GOAL_SCORE)
                endGame();
            else
                resetGameState();

        } else if (puck.x >= halfTableLength) {
            leftPlayerScore++;
            if (leftPlayerScore == GOAL_SCORE)
                endGame();
            else
                resetGameState();
        }
    }

    void resetGameState() {
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
        //TODO Show stuff. If not done here, remove method altogether, move state change to resetGameState
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage) {
        currentTime = std::chrono::system_clock::now();
        dt = 0.01f; /*std::chrono::duration<float, std::chrono::seconds::period>
            (currentTime - lastTime).count();*/

        switch (state) {
            case START:
                if (glfwGetKey(window, GLFW_KEY_P)) {
                    view = ABOVE;
                    resetGameState();
                }
                break;
            case RESET:
                if (glfwGetKey(window, GLFW_KEY_SPACE)) {
                    launchPuck();
                    state = PLAYING;
                }

                if (glfwGetKey(window, GLFW_KEY_V))
                    view = static_cast<views>((view + 1) % NUM_VIEWS + 1);
                break;
            case PLAYING:
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

                    debounceTime = currentTime;
                }

                puck.x += puck.vx * dt;
                puck.y += puck.vy * dt;

                checkScoreOccurred(); 

                if (glfwGetKey(window, GLFW_KEY_T)) { //Test setting, set to custom position
                    puck.x = halfTableLength - puckRadius - 0.2f;
                    puck.y = halfTableWidth - puckRadius - 0.2f;
                    launchPuck();
                }

                if (glfwGetKey(window, GLFW_KEY_V))
                    view = static_cast<views>((view + 1) % NUM_VIEWS + 1);
                break;

            case VICTORY:
                if (glfwGetKey(window, GLFW_KEY_R)) {
                    rightPlayerScore = 0;
                    leftPlayerScore = 0;
                    resetGameState();
                }
        }

        updateGPUData(currentImage);

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

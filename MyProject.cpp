// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

#define NUM_VIEWS 3
#define NUM_CORNERS 4

// The uniform buffer object used in this example
struct globalUniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
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

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M_Table;
	Texture T_Table;
	DescriptorSet DS_Table;	// instance DSLobj

	Model M_Puck;
	Texture T_Puck;
	DescriptorSet DS_Puck;	// instance DSLobj

	Model M_Paddle;
	Texture T_Paddle;
	DescriptorSet DS_LeftPaddle;	// instance DSLobj
	DescriptorSet DS_RightPaddle;	// instance DSLobj
	
	DescriptorSet DS_global;

	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 1800;
		windowHeight = 1000;
		windowTitle = "Air Hockey";
		initialBackgroundColor = {0.0f, 0.5f, 0.0f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 6;
		texturesInPool = 5;
		setsInPool = 6;
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
		
		M_Puck.init(this, "models/disk.obj");
		T_Puck.init(this, "textures/puck.png");
		DS_Puck.init(this, &DSLobj, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_Puck}
				});


		M_Paddle.init(this, "models/disk.obj"); //TODO Rename
		T_Paddle.init(this, "textures/paddle.png");
		DS_LeftPaddle.init(this, &DSLobj, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_Paddle}
				});
		DS_RightPaddle.init(this, &DSLobj, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_Paddle}
				});




		DS_global.init(this, &DSLglobal, {
					{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}
				});
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS_Table.cleanup();
		T_Table.cleanup();
		M_Table.cleanup();

		DS_Puck.cleanup();
		T_Puck.cleanup();
		M_Puck.cleanup();
		
		DS_LeftPaddle.cleanup();
		DS_RightPaddle.cleanup();
		M_Paddle.cleanup();
		T_Paddle.cleanup();

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

	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static auto lastTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::seconds::period>
			(currentTime - lastTime).count();

		//WTF is (*was) going on here with the time handling?

		//static int state = 0;		// 0 - everything is still.
									// 3 - three wheels are turning
									// 2 - two wheels are turning
									// 1 - one wheels is turning

		//static float debounce = time;

		float halfTableLength = 1.7428f/2; //TODO Modify according to the model
		float halfTableWidth = 0.451f; //TODO Modify according to the model
		float halfWholeTableHeight = 0.05f;
		float halfSideHeight = 0.018f;
		float halfTableHeight = halfWholeTableHeight - halfSideHeight;
		
		float cornerCircleRadius = halfTableWidth / 2;
		Point cornersCirclesCenters[NUM_CORNERS] = {
			{ -halfTableLength + cornerCircleRadius, -cornerCircleRadius }, //LEFT -> TOP
			{ -halfTableLength + cornerCircleRadius, cornerCircleRadius }, //		-> BOTTOM
			{ halfTableLength - cornerCircleRadius, -cornerCircleRadius }, //RIGHT -> TOP
			{ halfTableLength - cornerCircleRadius, cornerCircleRadius } //		-> BOTTOM
		};
		enum { LT, LB, RT, RB };
		
		float puckRadius = 0.0574f/2; //TODO Modify according to the model

		float paddleRadius = 0.0574f/2; //TODO Modify according to the model
		float paddleVelocity = 0.0001f;

		float scoreAreaX = 0.4f; //TODO ^^

		//Assumption: the table is centered in (0, 0)
		static Point lPaddle = { -halfTableLength + paddleRadius, 0.0f, 0.0f, 0.0f };
		static Point rPaddle = { halfTableLength - paddleRadius, 0.0f, 0.0f, 0.0f };
		static Point puck = { 0.0f, 0.0f, 0.0f, 0.0f };

		if (glfwGetKey(window, GLFW_KEY_SPACE)) { //START
			/*if (time - debounce > 0.15) {  Does it actually make sense implemented this way?
				debounce = time;*/
			if (puck.vx == 0.0f && puck.vy == 0.0f) {
				puck.vx = 0.00001f; //TODO make it random outside a cone
				puck.vy = 0.0001f;
			}
		}

		lPaddle.vx = 0.0f;
		lPaddle.vy = 0.0f;
		if (glfwGetKey(window, GLFW_KEY_A) /*TODO lPaddle not at the border*/) {
			lPaddle.vx -= paddleVelocity;
		}

		if (glfwGetKey(window, GLFW_KEY_D)) { //TODO same checks as above
			lPaddle.vx += paddleVelocity;
		}

		if (glfwGetKey(window, GLFW_KEY_W)) {
			lPaddle.vy -= paddleVelocity;
		}

		if (glfwGetKey(window, GLFW_KEY_S)) {
			lPaddle.vy += paddleVelocity;
		}

		rPaddle.vx = 0.0f;
		rPaddle.vy = 0.0f;
		if (glfwGetKey(window, GLFW_KEY_LEFT)) {  //TODO Same checks as above
			rPaddle.vx -= paddleVelocity;
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
			rPaddle.vx += paddleVelocity;
		}

		if (glfwGetKey(window, GLFW_KEY_UP)) {  //y-axis is reversed to match z-axis later during matrix creation
			rPaddle.vy -= paddleVelocity;
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN)) {
			rPaddle.vy += paddleVelocity;
		}

		
		float tempPosition;
		bool update = false;
		
		//TODO Refactor into single function
		tempPosition = lPaddle.x + lPaddle.vx * dt;
		if (tempPosition + paddleRadius <= -scoreAreaX) {
			if (abs(lPaddle.y) > cornerCircleRadius) {//LT, LR
				float relativeX = abs(tempPosition) - abs(cornersCirclesCenters[LT].x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
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
			if (abs(lPaddle.x) > abs(cornersCirclesCenters[LT].x)) {//LT, LR
				float relativeX = abs(lPaddle.x) - abs(cornersCirclesCenters[LT].x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
				float relativeY = abs(tempPosition) - cornerCircleRadius;
				float maxRelativeY = sqrt(pow((cornerCircleRadius - paddleRadius), 2) - pow(relativeX, 2));
				if (relativeY <= maxRelativeY)
					update = true;
			}
			else //TODO generalize if above using the absolute values of the quantities involved
				update = true;
		}
		if (update) {
			lPaddle.y = tempPosition;
			update = false;
		}
		
		
		tempPosition = rPaddle.x + rPaddle.vx * dt;
		if (tempPosition - paddleRadius >= scoreAreaX) {
			if (abs(rPaddle.y) > cornerCircleRadius) {//RT, RB
				float relativeX = abs(tempPosition) - abs(cornersCirclesCenters[RT].x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
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
			if (abs(rPaddle.x) > abs(cornersCirclesCenters[RT].x)) {//RT, RB
				float relativeX = abs(rPaddle.x) - abs(cornersCirclesCenters[RT].x); //Check to perform: relX < (cornerRadius - paddleRadius)cos alpha
				float relativeY = abs(tempPosition) - cornerCircleRadius;
				float maxRelativeY = sqrt(pow((cornerCircleRadius - paddleRadius), 2) - pow(relativeX, 2));
				if (relativeY <= maxRelativeY)
					update = true;
			}
			else //TODO generalize if above using the absolute values of the quantities involved
				update = true;
		}
		if (update)
			rPaddle.y = tempPosition;
		


		glm::vec2 normalVector;
		//Each case has to be handled separately otherwise it's not possible to determine which normal vector has to be used
		if (puck.y >= halfTableWidth - puckRadius) { //UPPER SIDE 
			normalVector = glm::vec2(0.0f, 1.0f);
			puck.vy *= -1; //TODO Quick trick to make it work now, generalize later
		}
		else if (puck.y <= -halfTableWidth + puckRadius) {
			normalVector = glm::vec2(0.0f, -1.0f); //LOWER SIDE
			puck.vy *= -1; //TODO ^^
		}
		//else if (/*curved pieces collision detected*/)
		//else if (/*paddle collision detected */ ) 
		
		//Calculate new vx, vy using the normal vector as done in the Phong model:
		//Normalize v, flip it (-1) and extract the modulus -> If defined separately at 266 (M * component), modulus is given  --> UPDATE: Normalization not required if normal is provided normalized (or is normalized below)
		//Dot product with normal, 2*result - initial normalized vector, times M

		//Handle case where nothing changes: setting a default normal and checking it probably the cheapest way?
			
		
		//if line was crossed
			//reset state
		//else update puck position
			puck.x += puck.vx * dt;
			puck.y += puck.vy * dt;


		globalUniformBufferObject gubo{};
		UniformBufferObject ubo{};
				
		void* data;

		float cameraHeight = 1.0f;
		glm::mat4 viewMatrices[NUM_VIEWS] = {
			glm::lookAt(glm::vec3(0.0f, 3.0f, 1.0f), //Center
						glm::vec3(0.0f, halfTableHeight, 0.0f),
						glm::vec3(0.0f, 1.0f, 0.0f)),
			glm::lookAt(glm::vec3(-halfTableLength - 0.7f, cameraHeight, 0.0f), //Left player
						glm::vec3(0.0f, halfTableHeight, 0.0f),
						glm::vec3(0.0f, 1.0f, 0.0f)),
			glm::lookAt(glm::vec3(halfTableLength + 0.7f, cameraHeight, 0.0f), //Right player
						glm::vec3(0.0f, halfTableHeight, 0.0f),
						glm::vec3(0.0f, 1.0f, 0.0f))

		};
		static unsigned viewIndex = 0;

		if (glfwGetKey(window, GLFW_KEY_V))
			viewIndex = (viewIndex + 1) % NUM_VIEWS;
		gubo.view = viewMatrices[viewIndex];

		gubo.proj = glm::perspective(glm::radians(45.0f),
						swapChainExtent.width / (float) swapChainExtent.height,
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
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(puck.x, 0.0f , puck.y));
					
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

	}	
};

// This is the main: probably you do not need to touch this!
int main() {
    MyProject app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
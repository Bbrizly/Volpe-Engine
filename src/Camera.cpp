// #include "Camera.h"
// #include <algorithm>

// using namespace Common;

// // Static singleton instance
// Camera* Camera::s_pCameraInstance = NULL;

// //------------------------------------------------------------------------------
// // Method:    CreateInstance
// // Returns:   void
// // 
// // Creates the singletone instance.
// //------------------------------------------------------------------------------
// void Camera::CreateInstance()
// {
// 	assert(s_pCameraInstance == NULL);
// 	s_pCameraInstance = new Camera();
// }

// //------------------------------------------------------------------------------
// // Method:    DestroyInstance
// // Returns:   void
// // 
// // Destroys the singleton instance.
// //------------------------------------------------------------------------------
// void Camera::DestroyInstance()
// {
// 	assert(s_pCameraInstance != NULL);
// 	delete s_pCameraInstance;
// 	s_pCameraInstance = NULL;
// }

// //------------------------------------------------------------------------------
// // Method:    Instance
// // Returns:   Camera::Camera*
// // 
// // Access to singleton instance.
// //------------------------------------------------------------------------------
// Camera* Camera::Instance()
// {
// 	assert(s_pCameraInstance);
// 	return s_pCameraInstance;
// }

// //------------------------------------------------------------------------------
// // Method:    Camera
// // Returns:   
// // 
// // Constructor.
// //------------------------------------------------------------------------------
// Camera::Camera()
// 	:
// 	m_pCamera(NULL),
// 	m_pLight(NULL)
// {
// 	m_pLight = new DirectionalLight();
// 	m_pLight->m_diffuse = volpe::Color4(1.0f,1.0f,1.0f,1.0f);
// 	m_pLight->m_ambient = volpe::Color4(0.3f,0.3f,0.3f,1.0f);
// 	m_pLight->m_specular = volpe::Color4(1.0f,1.0f,1.0f,1.0f);
// 	m_pLight->m_vDirection = glm::vec3(0.5f,-0.2f,-0.8f);
// }

// //------------------------------------------------------------------------------
// // Method:    ~Camera
// // Returns:   
// // 
// // Destructor.
// //------------------------------------------------------------------------------
// Camera::~Camera()
// {
// 	if (m_pLight)
// 	{
// 		delete m_pLight;
// 		m_pLight = NULL;
// 	}
// }

// //------------------------------------------------------------------------------
// // Method:    AddModel
// // Parameter: volpe::Model * p_pModel
// // Returns:   void
// // 
// // Adds a model the scene manager.
// //------------------------------------------------------------------------------
// void Camera::AddModel(volpe::Model* p_pModel)
// {
// 	m_lModelList.push_back(p_pModel);
// }

// //------------------------------------------------------------------------------
// // Method:    RemoveModel
// // Parameter: volpe::Model * p_pModel
// // Returns:   void
// // 
// // Removes a model from the scene manager.
// //------------------------------------------------------------------------------
// void Camera::RemoveModel(volpe::Model* p_pModel)
// {
// 	ModelList::iterator it = std::find(m_lModelList.begin(), m_lModelList.end(), p_pModel);
// 	if (it != m_lModelList.end())
// 	{
// 		m_lModelList.erase(it);
// 	}	
// }

// //------------------------------------------------------------------------------
// // Method:    Clear
// // Returns:   void
// // 
// // Clears the list of models in the scene manager.
// //------------------------------------------------------------------------------
// void Camera::Clear()
// {
// 	m_lModelList.clear();
// }

// //------------------------------------------------------------------------------
// // Method:    AddSprite
// // Parameter: volpe::Sprite * p_pSprite
// // Returns:   void
// // 
// // Adds the given sprite to the scene manager.
// //------------------------------------------------------------------------------
// void Camera::AddSprite(volpe::Sprite* p_pSprite)
// {
// 	m_lSpriteList.push_back(p_pSprite);
// }

// //------------------------------------------------------------------------------
// // Method:    RemoveSprite
// // Parameter: volpe::Sprite * p_pSprite
// // Returns:   void
// // 
// // Removes the given sprite from the scene manager.
// //------------------------------------------------------------------------------
// void Camera::RemoveSprite(volpe::Sprite* p_pSprite)
// {
// 	SpriteList::iterator it = std::find(m_lSpriteList.begin(), m_lSpriteList.end(), p_pSprite);
// 	if (it != m_lSpriteList.end())
// 	{
// 		m_lSpriteList.erase(it);
// 	}
// }

// //------------------------------------------------------------------------------
// // Method:    ClearSprites
// // Returns:   void
// // 
// // Clears the list of sprites in the scene manager.
// //------------------------------------------------------------------------------
// void Camera::ClearSprites()
// {
// 	m_lSpriteList.clear();
// }

// //------------------------------------------------------------------------------
// // Method:    AttachCamera
// // Parameter: Camera * p_pCamera
// // Returns:   void
// // 
// // Attaches the given camera to the scene
// //------------------------------------------------------------------------------
// void Camera::AttachCamera(Camera* p_pCamera)
// {
// 	m_pCamera = p_pCamera;
// }

// //------------------------------------------------------------------------------
// // Method:    GetCamera
// // Returns:   Camera*
// // 
// // Returns the active camera.
// //------------------------------------------------------------------------------
// Camera* Camera::GetCamera()
// {
// 	return m_pCamera;
// }

// void Camera::AddPointLight(const glm::vec3& position, const float range, const glm::vec3& color)
// {
// 	m_PointLights.push_back(PointLight(position, range, color));
// }

// void Camera::UpdatePointLight(const int index, const glm::vec3& position)
// {
// 	if (index >= 0 && index < m_PointLights.size())
// 	{
// 		m_PointLights[index].position = position;
// 	}

// 	//if (!m_PointLights.empty())
// 	//{
// 	//	m_PointLights[index].position = position; // Blue light follows the player
// 	//}
// }


// //------------------------------------------------------------------------------
// // Method:    Render
// // Returns:   void
// // 
// // Iterates the list of models, applies the camera params to the shader and 
// // renders the model.
// //------------------------------------------------------------------------------
// void Camera::Render()
// {
// 	// Can't render without a camera
// 	if (m_pCamera == NULL)
// 	{
// 		return;
// 	}

// 	// Get the view/proj matrices from the camera
// 	const glm::mat4& mProj = m_pCamera->GetProjectionMatrix();
// 	const glm::mat4& mView = m_pCamera->GetViewMatrix();

// 	// Iterate over the list of models and render them
// 	ModelList::iterator it = m_lModelList.begin(), end = m_lModelList.end();
// 	for (; it != end; ++it)
// 	{
// 		volpe::Model* pModel = static_cast<volpe::Model*>(*it);

// 		for (volpe::Material* pMaterial : pModel->GetMaterials())
// 		{
// 			float strength = 10.0f;

// 			if (m_PointLights.size() >= 2)
// 			{
// 				//glm::vec3 blueLightPos = glm::vec3(0.0f, 10.0f, 1.0f)/* get player position */;
// 				//glm::vec3 blueLightColor = glm::vec3(0.0f, 0.0f, 1.0f);
// 				pMaterial->SetUniform("blueLight.PositionRange", glm::vec4(m_PointLights[0].position, m_PointLights[0].range));//blueLightPos
// 				pMaterial->SetUniform("blueLight.Color", m_PointLights[0].color); //blueLightColor
// 				pMaterial->SetUniform("blueLight.Strength", strength);


// 				//glm::vec3 yellowLightPos = glm::vec3(0.0f, 5.0f, -10.0f); // Adjust as needed
// 				//glm::vec3 yellowLightColor = glm::vec3(1.0f, 1.0f, 0.0f);
// 				pMaterial->SetUniform("yellowLight.PositionRange", glm::vec4(m_PointLights[1].position, m_PointLights[1].range));//yellowLightPos
// 				pMaterial->SetUniform("yellowLight.Color", m_PointLights[1].color); //yellowLightColor
// 				pMaterial->SetUniform("yellowLight.Strength", strength);
// 			}

// 			// Set the light parameters
// 			//pMaterial->SetUniform("ViewDir", glm::vec3(0.0f, 0.0f, 1.0f));
// 			//pMaterial->SetUniform("LightAmbient", m_pLight->m_ambient);
// 			//pMaterial->SetUniform("LightDiffuse", m_pLight->m_diffuse);
// 			//pMaterial->SetUniform("LightSpecular", m_pLight->m_specular);
// 			//pMaterial->SetUniform("LightDir", m_pLight->m_vDirection);
// 		}

// 		pModel->Render(mView, mProj);
// 	}


// 	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 	// Render the sprite list with an ortho camera. 
// 	// TODO: We should really add the camera separately rather than hard code it.

// 	const glm::mat4 mOrthoProj = glm::ortho(0.0f,1280.0f,720.0f,0.0f,0.0f,1000.0f);
// 	SpriteList::iterator sIt = m_lSpriteList.begin(), sEnd = m_lSpriteList.end();
// 	for (; sIt != sEnd; ++sIt)
// 	{
// 		volpe::Sprite* pSprite = static_cast<volpe::Sprite*>(*sIt);
// 		pSprite->Render(mOrthoProj);
// 	}
// }

#pragma once

#include <vector>

#include "Component.h"
#include "mat.h"
#include "quaternion.h"

using namespace std;

/*!
*  \brief Classe g�rant un objet de la hi�rarchie de la sc�ne, ses composants et son Transform.
*/
class GameObject
{
private:
	string name = "GameObject";
	Transform translation = Identity();
	Transform rotation = Identity();
	Transform scale = Identity();
	Transform localTRS = Identity();
	Transform objectToWorld = Identity();
	TQuaternion<float, Vector> rotationQuat;

	vector<Component*> components;
	GameObject* parent = nullptr;
	vector<GameObject*> children;
	bool transformNeedsToUpdate = true;


public:
	GameObject()
	{
		rotationQuat = TQuaternion<float, Vector>(0, 0, 0, 1);
	}

	/*-------------Components-------------*/
	/*!
	*  \brief Ajout d'un composant au gameobject.
	*  \param component : le composant a ajouter.
	*/
	void AddComponent(Component* component)
	{
		component->SetGameObject(this);
		this->components.push_back(component);
	}

	/*!
	*  \brief Supprime un composant pass� en template (ex : gameobject.RemoveComponent<Component>()).
	*  \param component le composant a ajouter.
	*/
	template<typename T> void RemoveComponent()
	{
		for (int i = 0; i < components.size(); i++)
		{
			T* castAttempt = dynamic_cast<T*>(components[i]);
			if (castAttempt != nullptr)
				components.erase(components.begin() + i);
		}
		return nullptr;
	}

	/*!
	*  \brief R�cup�re un composant pass� en template  (ex : gameobject.GetComponent<Component>()).
	*/
	template<typename T> T* GetComponent()
	{
		for (int i = 0; i < components.size(); i++)
		{
			T* castAttempt = dynamic_cast<T*>(components[i]);
			if (castAttempt != nullptr)
				return castAttempt;
		}
		return nullptr;
	}

	/*!
	*  \brief R�cup�re tous les composant du gameObject.
	*  \return vector contenant les composant.
	*/
	vector<Component*> GetAllComponents()
	{
		return components;
	}
	/*-------------Components-------------*/

	/*-------------Children-------------*/
	/*!
	*  \brief Ajout d'un gamepbject enfant au gameobject.
	*  \param child le gameobject enfant a ajouter.
	*/
	void AddChild(GameObject* child)
	{
		child->SetParent(this);
		this->children.push_back(child);
	}

	/*!
	*  \brief Supprimer le gameobject enfant � la position i.
	*  \param i la position du gameobject � supprimer dans le vector de gameobject enfant.
	*/
	void RemoveChildAt(int i)
	{
		children.erase(children.begin() + i);
	}

	/*!
	*  \brief R�cup�re le gameobject enfant � la position i.
	*  \param i la position du gameobject � r�cup�rer dans le vector de gameobject enfant.
	*/
	GameObject* GetChildAt(int i)
	{
		return children[i];
	}

	/*!
	*  \brief R�cup�re tous les gameobjects enfants au gameobject.
	*/
	vector<GameObject*> GetAllChildren()
	{
		return children;
	}

	/*!
	*  \brief Attribue un parent au gameobject.
	*/
	void SetParent(GameObject* parent)
	{
		this->parent = parent;
	}

	/*!
	*  \brief R�cup�re le parent du gameobject.
	*/
	GameObject* GetParent()
	{
		return this->parent;
	}
	/*-------------Components-------------*/

	/*-------------Position Functions-------------*/
	/*!
	*  \brief R�cup�re la position du gameobject.
	*/
	Vector GetPosition()
	{
		return Vector(translation.m[0][3], translation.m[1][3], translation.m[2][3]);
	}

	/*!
	*  \brief Attribue la position locale du gameobject par rapport � son parent.
	*  \param : vector le vector3 de position � attribuer.
	*/
	void SetPosition(Vector vector)
	{
		translation.m[0][3] = vector.x;
		translation.m[1][3] = vector.y;
		translation.m[2][3] = vector.z;
		MarkTransformAsChanged();
	}

	/*!
	*  \brief Attribue la position locale du gameobject par rapport � son parent.
	*  \param : x la position en x � attribuer.
	*  \param : y la position en y � attribuer.
	*  \param : z la position en z � attribuer.
	*/
	void SetPosition(float x, float y, float z)
	{
		translation.m[0][3] = x;
		translation.m[1][3] = y;
		translation.m[2][3] = z;
		MarkTransformAsChanged();
	}
	/*-------------Position Functions-------------*/

	/*-------------Scale Functions-------------*/
	/*!
	*  \brief R�cup�re le scale local du gameobject.
	*/
	Vector GetScale()
	{
		return Vector(scale.m[0][0], translation.m[1][1], translation.m[2][2]);
	}

	/*!
	*  \brief Attribue le scale local du gameobject.
	*  \param : vector le vector3 de scale � attribuer.
	*/
	void SetScale(Vector vector)
	{
		scale.m[0][0] = vector.x;
		scale.m[1][1] = vector.y;
		scale.m[2][2] = vector.z;
		MarkTransformAsChanged();
	}

	/*!
	*  \brief Attribue le scale local du gameobject.
	*  \param : x le scale en x � attribuer.
	*  \param : y le scale en y � attribuer.
	*  \param : z le scale en z � attribuer.
	*/
	void SetScale(float x, float y, float z)
	{
		scale.m[0][0] = x;
		scale.m[1][1] = y;
		scale.m[2][2] = z;
		MarkTransformAsChanged();
	}
	/*-------------Scale Functions-------------*/

	/*-------------Rotation Functions-------------*/
	TQuaternion<float, Vector> GetRotation()
	{
		return rotationQuat;
	}

	/*!
	*  \brief Applique une rotation locale au gameobject.
	*  \param : quat la rotation en quaternion.
	*/
	void SetRotation(TQuaternion<float, Vector> quat)
	{
		rotationQuat = quat;
		const float* temp = rotationQuat.matrix();
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				rotation.m[i][j] = temp[i * 4 + j];
			}
		}
		/*rotation.m[2][0] = -rotation.m[2][0];
		rotation.m[2][1] = -rotation.m[2][1];
		rotation.m[2][2] = -rotation.m[2][2];*/
		MarkTransformAsChanged();
	}

	/*!
	*  \brief Applique une rotation locale en degr� autour d'un axe.
	*  \param : axis l'axe autour duquel la rotation aura lieu.
	*  \param : degrees la roatation en degr�.
	*/
	void RotateAround(Vector axis, float degrees)
	{
		degrees *= 0.0174533f;
		TQuaternion<float, Vector> quat = TQuaternion<float, Vector>(axis, degrees);
		SetRotation(rotationQuat * quat);
	}

	/*!
	*  \brief Applique une rotation locale en radian autour d'un axe.
	*  \param : axis l'axe autour duquel la rotation aura lieu.
	*  \param : degree la roatation en radian.
	*/
	void RotateAroundRadian(Vector axis, float radian)
	{
		TQuaternion<float, Vector> quat = TQuaternion<float, Vector>(axis, radian);
		SetRotation(rotationQuat * quat);
	}

	/*!
	*  \brief Applique une rotation pour que le vecteur Forward du gameobject regarde dans la direction d'une position.
	*  \param : destPoint la position � regarder.
	*/
	void LookAt(Vector destPoint)
	{
		Vector newForward = normalize(GetPosition() - destPoint);

		float dotValue = dot(GetForwardVector(), newForward);
		if (dotValue > 1.0f)
			dotValue = 1.0f;

		if (abs(dotValue + 1.0f) < 0.000001f)
		{
			RotateAroundRadian(GetUpVector(), 3.14159265f);
			return;
		}

		if (abs(dotValue - 1.0f) < 0.000001f)
		{
			return;
		}

		float rotAngle = (float)acos(dotValue);
		Vector rotAxis = cross(GetForwardVector(), newForward);
		rotAxis = normalize(rotAxis);
		RotateAroundRadian(rotAxis, -rotAngle);
	}

	/*!
	*  \brief Applique une rotation pour que le vecteur Up du gameobject regarde dans la direction d'une position.
	*  \param : destPoint la position � regarder.
	*/
	void LookAtUpVector(Vector destPoint)
	{
		Vector newForward = normalize(GetPosition() - destPoint);

		float dotValue = dot(-GetUpVector(), newForward);
		if (dotValue > 1.0f)
			dotValue = 1.0f;

		if (abs(dotValue + 1.0f) < 0.000001f)
		{
			RotateAroundRadian(GetRightVector(), 3.14159265f);
			return;
		}

		if (abs(dotValue - 1.0f) < 0.000001f)
		{
			return;
		}

		float rotAngle = (float)acos(dotValue);
		Vector rotAxis = cross(-GetUpVector(), newForward);
		rotAxis = normalize(rotAxis);
		RotateAroundRadian(rotAxis, -rotAngle);
	}
	/*-------------Rotation Functions-------------*/


	/*-------------Transform Management-------------*/
	/*!
	*  \brief R�cup�re le vector right de la rotation du gameobject.
	*/
	Vector GetRightVector()
	{
		return rotation[0];
	}

	/*!
	*  \brief R�cup�re le vector up de la rotation du gameobject.
	*/
	Vector GetUpVector()
	{
		return rotation[1];
	}

	/*!
	*  \brief R�cup�re le vector forward de la rotation du gameobject.
	*/
	Vector GetForwardVector()
	{
		return rotation[2];
	}

	/*!
	*  \brief R�cup�re la matrice TRS (translation, rotation, scale).
	*/
	Transform GetTRS()
	{
		return localTRS;
	}

	/*!
	*  \brief R�cup�re la matrice Objet->Monde du gameobject.
	*/
	Transform GetObjectToWorldMatrix()
	{
		return objectToWorld;
	}

	/*!
	*  \brief Averti que le transform du gameobject a chang� ainsi que pour ses enfants.
	*	Ceci d�clenche une mise � jour de toutes les matrices Objet->Monde de ce GameObject
	*	et de ses enfants.
	*/
	void MarkTransformAsChanged()
	{
		transformNeedsToUpdate = true;
		for (int i = 0; i < children.size(); i++)
			children[i]->MarkTransformAsChanged();
	}

	/*!
	*  \brief Met � jour la matrice Objet->Monde si le transform a �t� modifi�.
	*/
	void UpdateTransformIfNeeded()
	{
		if (transformNeedsToUpdate)
		{
			// Update local TRS matrix
			localTRS = translation * rotation * scale;

			// Update object to world matrix
			GameObject* temp = parent;
			objectToWorld = Identity();
			while (temp != nullptr && temp->transformNeedsToUpdate == true)
			{
				temp->UpdateTransformIfNeeded();
				temp = temp->GetParent();
			}

			if (this->parent != nullptr)
				objectToWorld = this->parent->GetObjectToWorldMatrix() * localTRS;
			else
				objectToWorld = localTRS;
			transformNeedsToUpdate = false;
		}
	}
	/*-------------Transform Management-------------*/

	/*!
	*  \brief Attribue un nom au gameobject.
	*  \param : string, le nom � attribuer au gameobjet.
	*/
	void SetName(string s)
	{
		name = s;
	}

	/*!
	*  \brief R�cup�re le nom du gameobject.
	*/
	string GetName()
	{
		return name;
	}
};

#include "Core/Misc/SafeDestroy.h"
#include "Render/Shader.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterInstance.h"
#include "Spray/EmitterUpdateContext.h"
#include "Spray/Modifier.h"
#include "Spray/PointRenderer.h"
#include "Spray/Source.h"

#if defined(T_MODIFIER_USE_PS3_SPURS)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/SprayJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"
#endif

#if !TARGET_OS_IPHONE && !defined(_WINCE)
#	define T_USE_UPDATE_JOBS
#endif

namespace traktor
{
	namespace spray
	{
		namespace
		{

const float c_warmUpDeltaTime = 1.0f / 10.0f;
#if TARGET_OS_IPHONE
const uint32_t c_maxEmitPerUpdate = 4;
const uint32_t c_maxEmitSingleShot = 10;
#elif defined(_PS3)
const uint32_t c_maxEmitPerUpdate = 8;
const uint32_t c_maxEmitSingleShot = 2000;
#else
const uint32_t c_maxEmitPerUpdate = 8;
const uint32_t c_maxEmitSingleShot = 3000;
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EmitterInstance", EmitterInstance, Object)

EmitterInstance::EmitterInstance(Emitter* emitter)
:	m_emitter(emitter)
,	m_totalTime(0.0f)
,	m_emitted(0)
,	m_warm(false)
,	m_count(std::rand() & 15)
{
	// Pre-allocate points; estimate average required.
	m_points.reserve(std::max(c_maxEmitPerUpdate * 30 * 10, c_maxEmitSingleShot));
}

EmitterInstance::~EmitterInstance()
{
	synchronize();
}

void EmitterInstance::update(EmitterUpdateContext& context, const Transform& transform, bool emit, bool singleShot)
{
	// Warm up instance.
	if (!m_warm)
	{
		m_warm = true;
		if (m_emitter->getWarmUp() >= FUZZY_EPSILON)
		{
			EmitterUpdateContext warmContext(c_warmUpDeltaTime);

			float time = 0.0f;
			for (; time < m_emitter->getWarmUp(); time += c_warmUpDeltaTime)
				update(warmContext, transform, true, false);

			warmContext.deltaTime = m_emitter->getWarmUp() - time;
			if (warmContext.deltaTime >= FUZZY_EPSILON)
				update(warmContext, transform, true, false);
		}
	}

	synchronize();

	// Erase dead particles.
	size_t size = m_points.size();
	for (size_t i = 0; i < size; )
	{
		Point& point = m_points[i];
		if ((point.age += context.deltaTime) < point.maxAge)
			++i;
		else if (i < --size)
			point = m_points[size];
	}
	m_points.resize(size);

	// Emit particles.
	if (emit)
	{
		Source* source = m_emitter->getSource();
		if (source)
		{
			m_totalTime += context.deltaTime;
			if (!singleShot)
			{
				// Emit in multiple frames; estimate number of particles to emit.
				uint32_t goal = uint32_t(m_totalTime * source->getRate());
				uint32_t emitCount = std::min< uint32_t >(goal - m_emitted, c_maxEmitPerUpdate);
				if (emitCount > 0)
				{
					m_points.reserve(size + emitCount);
					source->emit(
						context,
						transform,
						emitCount,
						*this
					);
				}
			}
			else
			{
				// Single shot emit; emit all particles in one frame and then no more.
				uint32_t emitCount = std::min< uint32_t >(uint32_t(source->getRate()), c_maxEmitSingleShot);
				m_points.reserve(size + emitCount);
				source->emit(
					context,
					transform,
					emitCount,
					*this
				);
			}
		}
	}

	// Calculate bounding box; do this before modifiers as modifiers are executed
	// asynchronously.
	if ((m_count++ & 15) == 0)
	{
		m_boundingBox = Aabb();
		Scalar deltaTime16 = Scalar(context.deltaTime * 16.0f);
		for (PointVector::iterator i = m_points.begin(); i != m_points.end(); ++i)
			m_boundingBox.contain(i->position + i->velocity * deltaTime16);
	}

#if defined(T_MODIFIER_USE_PS3_SPURS)
	//
	// Update particles on SPU
	//
	if (!m_points.empty())
	{
		SpursJobQueue* jobQueue = SprayJobQueue::getInstance().getJobQueue();
		T_ASSERT (jobQueue);

		const RefArray< Modifier >& modifiers = m_emitter->getModifiers();
		for (RefArray< Modifier >::const_iterator i = modifiers.begin(); i != modifiers.end(); ++i)
		{
			if (*i)
				(*i)->update(
					jobQueue,
					Scalar(context.deltaTime),
					transform,
					m_points
				);
		}
	}
#else
	//
	// Update particles on CPU
	//
#	if defined(T_USE_UPDATE_JOBS)
	// Execute modifiers.
	if (size >= 4)
	{
		size_t pivots[] =
		{
			0,
			size / 4,
			(size * 2) / 4,
			(size * 3) / 4,
			size
		};

		JobManager& jobManager = JobManager::getInstance();
		m_jobs[0] = jobManager.add(makeFunctor< EmitterInstance, float, const Transform&, size_t, size_t >(this, &EmitterInstance::updateTask, context.deltaTime, transform, pivots[0], pivots[1]));
		m_jobs[1] = jobManager.add(makeFunctor< EmitterInstance, float, const Transform&, size_t, size_t >(this, &EmitterInstance::updateTask, context.deltaTime, transform, pivots[1], pivots[2]));
		m_jobs[2] = jobManager.add(makeFunctor< EmitterInstance, float, const Transform&, size_t, size_t >(this, &EmitterInstance::updateTask, context.deltaTime, transform, pivots[2], pivots[3]));
		m_jobs[3] = jobManager.add(makeFunctor< EmitterInstance, float, const Transform&, size_t, size_t >(this, &EmitterInstance::updateTask, context.deltaTime, transform, pivots[3], pivots[4]));
	}
#	else
	updateTask(context.deltaTime, transform, 0, m_points.size());
#	endif
#endif
}

void EmitterInstance::render(PointRenderer* pointRenderer, const Plane& cameraPlane) const
{
	//synchronize();

	if (m_points.empty())
		return;

	resource::Proxy< render::Shader >& shader = m_emitter->getShader();
	if (!shader.validate())
		return;

	pointRenderer->render(
		shader,
		cameraPlane,
		m_points,
		m_emitter->getMiddleAge(),
		m_emitter->getCullNearDistance(),
		m_emitter->getFadeNearRange()
	);
}

void EmitterInstance::synchronize() const
{
#if defined(T_MODIFIER_USE_PS3_SPURS)

	SpursJobQueue* jobQueue = SprayJobQueue::getInstance().getJobQueue();
	T_ASSERT (jobQueue);

	jobQueue->wait();

#else

#	if defined(T_USE_UPDATE_JOBS)
	for (int i = 0; i < 4; ++i)
	{
		if (m_jobs[i])
		{
			m_jobs[i]->wait();
			m_jobs[i] = 0;
		}
	}
#	endif

#endif
}

#if !defined(T_MODIFIER_USE_PS3_SPURS)
void EmitterInstance::updateTask(float deltaTime, const Transform& transform, size_t first, size_t last)
{
	Scalar deltaTimeScalar(deltaTime);
	const RefArray< Modifier >& modifiers = m_emitter->getModifiers();
	for (RefArray< Modifier >::const_iterator i = modifiers.begin(); i != modifiers.end(); ++i)
	{
		if (*i)
			(*i)->update(deltaTimeScalar, transform, m_points, first, last);
	}
}
#endif

	}
}

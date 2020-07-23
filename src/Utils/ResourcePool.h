#ifndef MYUTILS_RESOURCEPOOL_H
#define MYUTILS_RESOURCEPOOL_H

#include <atomic>
#include <memory>
#include <mutex>
#include <deque>

using namespace std;

namespace toolkit
{

template<typename T>
class ResourcePoolImpl;

template<typename T>
class SharedPtrRes : public std::shared_ptr<T>
{
public:
    typedef std::weak_ptr<ResourcePoolImpl<T>> WeakResPoolImplPtr;
    typedef std::shared_ptr<ResourcePoolImpl<T>> ResPoolImplPtr;

    SharedPtrRes(T *ptr,
                 const WeakResPoolImplPtr &weak_res_pool_impl,
                 const std::shared_ptr<atomic_bool> &if_force_quit)
            : std::shared_ptr<T>(ptr,
                              [weak_res_pool_impl, if_force_quit](T *ptr)
                              {
                                  ResPoolImplPtr strong_pool_impl(weak_res_pool_impl.lock());
                                  if (strong_pool_impl && !if_force_quit)
                                      // 资源没被舍弃,尝试重新放入
                                      strong_pool_impl->release(ptr);
                                  else
                                      delete ptr;
                              }) {}


    void quit(bool if_quit = true)
    {
        if (if_force_quit_)
            *if_force_quit_ = if_quit;
    }

    SharedPtrRes() = default;

private:
    std::shared_ptr<atomic_bool> if_force_quit_;
};

// 循环资源池实现类
template<typename T>
class ResourcePoolImpl : public std::enable_shared_from_this<ResourcePoolImpl<T> >
{
public:
    typedef SharedPtrRes<T> ResPtr;

    ResourcePoolImpl()
    {
        t_functor_ = []()
        {
            return new T();
        };
        printf("ResourcePoolImpl Constructed!\n");
    }

    template<typename ...ArgTypes>
    explicit ResourcePoolImpl(ArgTypes &&...args)
    {
        t_functor_ = [args...]() -> T *
        {
            return new T(args...);
        };
        printf("ResourcePoolImpl ArgTypes Constructed!\n");
    }

    ~ResourcePoolImpl()
    {
        std::lock_guard<std::mutex> lck(pool_mutex_);
        for (auto res : res_pool_)
        {
            delete res;
            res = nullptr;
        };
    }

    void set_pool_size(uint size)
    {
        pool_size_ = size;
    }

    ResPtr obtain_res()
    {
        std::lock_guard<std::mutex> lck(pool_mutex_);
        T *ptr;
        if (res_pool_.empty())
            ptr = t_functor_();
        else
        {
            ptr = res_pool_.front();
            res_pool_.pop_front();
        }

        std::weak_ptr<ResourcePoolImpl<T>> weak_self = this->shared_from_this();
        return ResPtr(ptr, weak_self, std::make_shared<atomic_bool>(false));
    }

    void release(T *res)
    {
        std::lock_guard<decltype(pool_mutex_)> lck(pool_mutex_);
        if (res_pool_.size() >= pool_size_)
        {
            delete res;
            return;
        }
        res_pool_.emplace_back(res);
    }

private:
    std::function<T *(void)> t_functor_;
    list<T *> res_pool_;
    std::mutex pool_mutex_;
    uint pool_size_ = 10;
};

// 循环资源池
template<typename T>
class ResourcePool
{
public:
    typedef SharedPtrRes<T> ResPtr;

    ResourcePool()
    {
        pool_impl_.reset(new ResourcePoolImpl<T>());
    }

    template<typename ...ArgTypes>
    explicit ResourcePool(ArgTypes &&...args)
    {
        pool_impl_ = std::make_shared<ResourcePoolImpl<T> >(std::forward<ArgTypes>(args)...);
    }

    void set_pool_size(int size)
    {
        pool_impl_->set_pool_size(size);
    }

    ResPtr obtain()
    {
        return pool_impl_->obtain_res();
    }

private:
    std::shared_ptr<ResourcePoolImpl<T> > pool_impl_;
};

}

#endif //MYUTILS_RESOURCEPOOL_H

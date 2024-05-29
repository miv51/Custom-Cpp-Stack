
//custom queue class with performance improvements over std::queue and std::deque

#ifndef SQ_UTILS_H
#define SQ_UTILS_H

#include <stdexcept>

/*
A queue class that dynamically grows and shrinks in size by adding and removing blocks of memory at a time instead of doing that with single elements
Has iterators that transverse and construct faster than those for std::deque and std::queue - pushing and popping are also considerably faster (even with O2 optimization)
Can take up significantly less (or more) memory based on the chosen stack size - a stack size of 1 will take up the most memory and probably more than std::queue and std::deque
*/

template <typename dataType, size_t block_size>
class segmentedQueue
{
public:
    class segmentedQueueIterator;
    class segmentedQueueReverseIterator;

    segmentedQueue(const segmentedQueue&); //deep copy constructor

    segmentedQueue();
    ~segmentedQueue();
    
    constexpr inline const segmentedQueueReverseIterator rbegin() const { return segmentedQueueReverseIterator(*this); }
    constexpr inline const segmentedQueueReverseIterator rend() const { return segmentedQueueReverseIterator(); }
    constexpr inline const segmentedQueueIterator begin() const { return segmentedQueueIterator(*this); }
    constexpr inline const segmentedQueueIterator end() const { return segmentedQueueIterator(); }
    constexpr inline const size_t size() const noexcept { return length; }

    constexpr inline void push_back(const dataType&); //place an item in front of the most recent element - making it the first element
    constexpr inline void pop_front(); //remove the oldest element in the queue
    constexpr inline void clear(); //empty the queue

    constexpr inline const dataType& front() const; //access the oldest element in the queue
    constexpr inline dataType& operator[](size_t); //access elements in the order that they were added (0 is the oldest element)
    constexpr inline segmentedQueue& operator=(const segmentedQueue&); //assignment operator

private:
    struct stack;

    inline void reset(); //helper function - resets the first stack and length of the queue

    stack* first;
    stack* last;

    size_t length;
};

template <typename dataType, size_t block_size>
struct segmentedQueue<dataType, block_size>::stack
{
    stack* next = nullptr;
    stack* previous = nullptr;

    size_t length = 0;
    size_t start = 0;

    dataType data[block_size];
};

template <typename dataType, size_t block_size>
class segmentedQueue<dataType, block_size>::segmentedQueueIterator
{
public:
    segmentedQueueIterator(const segmentedQueue&);
    segmentedQueueIterator();

    ~segmentedQueueIterator();

    segmentedQueueIterator& operator++(); //pre-increment operator
    segmentedQueueIterator operator++(int); //post-increment operator
    segmentedQueueIterator& operator--(); //pre-deincrement operator
    segmentedQueueIterator operator--(int); //post-deincrement operator
    
    //de-reference operators
    dataType& operator*() const;
    dataType* operator->() const;

    bool operator==(const segmentedQueueIterator&) const noexcept; //equality operator
    bool operator!=(const segmentedQueueIterator&) const noexcept; //inequality operator
    
private:
    stack* current = nullptr;
    size_t position = 0;
};

template <typename dataType, size_t block_size>
class segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator
{
public:
    segmentedQueueReverseIterator(const segmentedQueue&);
    segmentedQueueReverseIterator();

    ~segmentedQueueReverseIterator();

    segmentedQueueReverseIterator& operator++(); //pre-increment operator
    segmentedQueueReverseIterator operator++(int); //post-increment operator
    segmentedQueueReverseIterator& operator--(); //pre-deincrement operator
    segmentedQueueReverseIterator operator--(int); //post-deincrement operator

    //de-reference operators
    dataType& operator*() const;
    dataType* operator->() const;

    bool operator==(const segmentedQueueReverseIterator&) const noexcept; //equality operator
    bool operator!=(const segmentedQueueReverseIterator&) const noexcept; //inequality operator

private:
    stack* current = nullptr;
    size_t position = 0;
};

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueue(const segmentedQueue& other)
{
    first = new stack();
    last = first;

    reset();

    stack* temp = other.last;

    while (temp != nullptr)
    {
        //not fastest way to copy elements but this won't be used on the critical path
        for (size_t index = 0; index < temp->length; ++index) push_back(temp->data[(temp->start + index) % block_size]);

        temp = temp->next;
    }
}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueue()
{
    first = new stack();
    last = first;

    reset();
}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::~segmentedQueue()
{
    clear();

    delete first;

    first = nullptr;
    last = nullptr;
}

template <typename dataType, size_t block_size>
constexpr inline void segmentedQueue<dataType, block_size>::push_back(const dataType& item)
{
    if (first->length >= block_size) //first should NEVER be nullptr
    {
        stack* temp = first;

        first = new stack();

        first->length = 0;
        first->start = 0;

        temp->next = first;

        first->previous = temp;
    }

    first->data[(first->start + first->length++) % block_size] = item;

    ++length;
}

template <typename dataType, size_t block_size>
constexpr inline void segmentedQueue<dataType, block_size>::pop_front()
{
    if (length <= 0) throw std::runtime_error("Cannot pop from empty segmented queue.");
    if (last->length <= 1 && last->next != nullptr) //don't remove temp if it is the only stack in the queue
    {
        stack* temp = last;

        last = last->next; //last should NEVER be nullptr

        last->previous = nullptr;

        delete temp;
    }
    else
    {
        last->start = (last->start + 1) % block_size;
        last->length--;
    }

    --length;
}

template <typename dataType, size_t block_size>
constexpr inline void segmentedQueue<dataType, block_size>::clear()
{
    while (last != first)
    {
        stack* temp = last;

        last = last->next;
        last->previous = nullptr;

        delete temp;
    }

    reset();
}

template <typename dataType, size_t block_size>
constexpr inline const dataType& segmentedQueue<dataType, block_size>::front() const
{
    if (length <= 0) throw std::runtime_error("Cannot peek in empty segmented queue.");

    return last->data[last->start];
}

template <typename dataType, size_t block_size>
constexpr inline dataType& segmentedQueue<dataType, block_size>::operator[](size_t index)
{
    stack* temp = last;

    if (index >= length) throw std::runtime_error("Index out of range for segmented queue.");
    while (index >= temp->length) { index -= temp->length; temp = temp->next; }
    return temp->data[(temp->start + index) % block_size];
}

template <typename dataType, size_t block_size>
constexpr inline segmentedQueue<dataType, block_size>& segmentedQueue<dataType, block_size>::operator=(const segmentedQueue& other)
{
    clear();

    stack* temp = other.last;

    while (temp != nullptr)
    {
        //not fastest way to copy elements but this won't be used on the critical path
        for (size_t index = 0; index < temp->length; ++index) push_back(temp->data[(temp->start + index) % block_size]);

        temp = temp->next;
    }

    return *this;
}

template <typename dataType, size_t block_size>
inline void segmentedQueue<dataType, block_size>::reset()
{
    first->previous = nullptr;
    first->next = nullptr;

    first->start = 0;
    first->length = 0;

    length = 0;
}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueIterator::segmentedQueueIterator(const segmentedQueue& segmented_queue) : current(segmented_queue.last) {}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueIterator::segmentedQueueIterator() {}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueIterator::~segmentedQueueIterator() {}

template <typename dataType, size_t block_size>
typename segmentedQueue<dataType, block_size>::segmentedQueueIterator& segmentedQueue<dataType, block_size>::segmentedQueueIterator::operator++()
{
    if (current == nullptr) throw std::runtime_error("End of segmented queue iterator.");
    if (position + 1 >= current->length) { current = current->next; position = 0; }
    else ++position;

    return *this;
}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueIterator segmentedQueue<dataType, block_size>::segmentedQueueIterator::operator++(int)
{
    throw std::runtime_error("Post-increment operator for segmented queue iterator not implemented.");
    return *this;
}

template <typename dataType, size_t block_size>
typename segmentedQueue<dataType, block_size>::segmentedQueueIterator& segmentedQueue<dataType, block_size>::segmentedQueueIterator::operator--()
{
    throw std::runtime_error("Post-increment operator for segmented queue iterator not implemented.");
    return *this;
}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueIterator segmentedQueue<dataType, block_size>::segmentedQueueIterator::operator--(int)
{
    throw std::runtime_error("Post-increment operator for segmented queue iterator not implemented.");
    return *this;
}

template <typename dataType, size_t block_size>
dataType& segmentedQueue<dataType, block_size>::segmentedQueueIterator::operator*() const
{
    if (current == nullptr || position >= current->length) throw std::runtime_error("Attempting to dereference an invalid segmented queue iterator.");
    return current->data[(current->start + position) % block_size];
}

template <typename dataType, size_t block_size>
dataType* segmentedQueue<dataType, block_size>::segmentedQueueIterator::operator->() const
{
    if (current == nullptr || position >= current->length) throw std::runtime_error("Attempting to dereference an invalid segmented queue iterator.");
    return &(current->data[(current->start + position) % block_size]);
}

template <typename dataType, size_t block_size>
bool segmentedQueue<dataType, block_size>::segmentedQueueIterator::operator==(const segmentedQueueIterator& other) const noexcept
{
    if (current == nullptr && other.current == nullptr) return true;
    if (current != other.current) return false;

    return position == other.position;
}

template <typename dataType, size_t block_size>
bool segmentedQueue<dataType, block_size>::segmentedQueueIterator::operator!=(const segmentedQueueIterator& other) const noexcept
{
    if (current == nullptr && other.current == nullptr) return false;
    if (current != other.current) return true;

    return position != other.position;
}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::segmentedQueueReverseIterator(const segmentedQueue& segmented_queue)
    : current(segmented_queue.first),
    position(segmented_queue.first->length)
{
    if (position == 0) current = nullptr;
    else --position;
}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::segmentedQueueReverseIterator() {}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::~segmentedQueueReverseIterator() {}

template <typename dataType, size_t block_size>
typename segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator& segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::operator++()
{
    if (current == nullptr) throw std::runtime_error("End of segmented queue reverse iterator.");
    if (position == 0)
    {
        current = current->previous;

        if (current == nullptr) return *this; //return if we reached the end of the iterator

        position = current->length;

        if (position == 0) throw std::runtime_error("Segmented queues should have at most one empty stack.");
    }

    --position;

    return *this;
}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::operator++(int)
{
    throw std::runtime_error("Post-increment operator for segmented queue reverse iterator not implemented.");
    return *this;
}

template <typename dataType, size_t block_size>
typename segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator& segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::operator--()
{
    throw std::runtime_error("Pre-deincrement operator for segmented queue reverse iterator not implemented.");
    return *this;
}

template <typename dataType, size_t block_size>
segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::operator--(int)
{
    throw std::runtime_error("Post-deincrement operator for segmented queue reverse iterator not implemented.");
    return *this;
}

template <typename dataType, size_t block_size>
dataType& segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::operator*() const
{
    if (current == nullptr || position > current->length) throw std::runtime_error("Attempting to dereference an invalid segmented queue reverse iterator.");
    return current->data[(current->start + position) % block_size];
}

template <typename dataType, size_t block_size>
dataType* segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::operator->() const
{
    if (current == nullptr || position > current->length) throw std::runtime_error("Attempting to dereference an invalid segmented queue reverse iterator.");
    return &(current->data[(current->start + position) % block_size]);
}

template <typename dataType, size_t block_size>
bool segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::operator==(const segmentedQueueReverseIterator& other) const noexcept
{
    if (current == nullptr && other.current == nullptr) return true;
    if (current != other.current) return false;

    return position == other.position;
}

template <typename dataType, size_t block_size>
bool segmentedQueue<dataType, block_size>::segmentedQueueReverseIterator::operator!=(const segmentedQueueReverseIterator& other) const noexcept
{
    if (current == nullptr && other.current == nullptr) return false;
    if (current != other.current) return true;

    return position != other.position;
}

#endif

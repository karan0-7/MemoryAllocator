#include <iostream>
#include <sys/mman.h>
#include <stdexcept>
#include <cstring>

const int MAGIC_NUMBER = 100;
const int HEAP_SIZE = 4096;

struct Header
{
    int size{};
    int magicNumber{};
};

struct Node
{
    int totalBytesAvailable{};
    Node *next{nullptr};
};

class MemAlloc
{
private:
    Node *head{};
    Node *tail{};

    void initiateList()
    {
        void *ptr = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (ptr == MAP_FAILED)
        {
            perror("mmap");
            throw std::runtime_error(std::strerror(errno));
        }

        Node *node = reinterpret_cast<Node *>(ptr);

        node->totalBytesAvailable = HEAP_SIZE - sizeof(Node);
        node->next = nullptr;
        head = node;
        tail = node;
    }

    void updateCurrentNodeAddressChange(Node *startingAddress, int availableBytes, Node *next, Node *parentNode)
    {

        Node *childAddress = startingAddress;
        if (availableBytes > sizeof(Node))
        {
            int bytesCanBeAllocated = availableBytes - sizeof(Node);
            startingAddress->totalBytesAvailable = bytesCanBeAllocated;
            startingAddress->next = next;
        }
        else
        {
            childAddress = next;
        }

        if (parentNode)
        {
            parentNode->next = childAddress;
        }`
        else
        {
            head = childAddress;
            tail = childAddress;
        }
    }

public:
    void *allocate(int bytesToAllocate)
    {

        if (bytesToAllocate <= 0)
        {
            throw std::runtime_error("Invalid input");
        }

        if (!head)
        {
            initiateList();
        }

        int totalBytesRequired = sizeof(Header) + bytesToAllocate;
        Node *currentNode = head;
        Node *parentNode = nullptr;

        while (currentNode)
        {
            int totalAvailableBytes = sizeof(Node) + currentNode->totalBytesAvailable;

            if (totalBytesRequired > totalAvailableBytes)
            {
                parentNode = currentNode;
                currentNode = currentNode->next;
            }
            else
            {
                Node currentNodeCopy = *currentNode;

                int totalAllocatedBlockSize = sizeof(Header) + bytesToAllocate;

                Node *updatedCurrentNodeAddress = reinterpret_cast<Node *>(reinterpret_cast<char *>(currentNode) + totalAllocatedBlockSize);
                int newAvailableBytes = totalAvailableBytes - totalBytesRequired;
                updateCurrentNodeAddressChange(updatedCurrentNodeAddress, newAvailableBytes, currentNodeCopy.next, parentNode);

                Header *header = reinterpret_cast<Header *>(currentNode);
                header->size = bytesToAllocate;
                header->magicNumber = MAGIC_NUMBER;

                void *allocatedAddress = reinterpret_cast<char *>(header) + sizeof(Header);

                return allocatedAddress;
            }
        }

        return nullptr;
    }

    void deallocate(void *ptr)
    {
        Header *headerStart = reinterpret_cast<Header *>(reinterpret_cast<char *>(ptr) - sizeof(Header));
        int totalAvailableBytes = sizeof(Header) + headerStart->size;

        if (totalAvailableBytes > sizeof(Node))
        {
            Node *node = reinterpret_cast<Node *>(headerStart);
            node->totalBytesAvailable = totalAvailableBytes - sizeof(Node);
            node->next = nullptr;
            tail->next = node;
            tail = node;
        }
    }
};

int main()
{
    MemAlloc alloc;
    alloc.allocate(20);
    alloc.allocate(20);
    alloc.allocate(20);

    return 0;
}
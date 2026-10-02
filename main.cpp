#include <iostream>
#include <sys/mman.h>

const int MAGIC_NUMBER = 100;

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

    void initiateList()
    {
        std::cout << "head" << head << '\n';

        void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        std::cout << "ptr" << ptr << '\n';

        Node *node = reinterpret_cast<Node *>(ptr);

        std::cout << "node" << node << '\n';

        node->totalBytesAvailable = 4096 - sizeof(Node);
        node->next = nullptr;
        head = node;
        std::cout << "head" << head << '\n';
    }

public:
    void *allocate(int bytes)
    {
        if (!head)
        {
            initiateList();
        }
        int totalBytesRequired = bytes + sizeof(Header);
        Node *currentChunk = head;

        std::cout << "currentChunk" << currentChunk << '\n';

        while (currentChunk)
        {
            int availableBytes = currentChunk->totalBytesAvailable;
            std::cout << "availableBytes" << availableBytes << '\n';
            std::cout << "totalBytesRequired" << totalBytesRequired << '\n';

            if (totalBytesRequired > availableBytes)
            {
                currentChunk = currentChunk->next;
            }
            else
            {
                Header *header = reinterpret_cast<Header *>(currentChunk);
                header->size = bytes;
                header->magicNumber = MAGIC_NUMBER;
                std::cout << "header" << header << '\n';
                std::cout << "sizeof(Header)" << sizeof(Header) << '\n';
                void *allocatedAddress = header + sizeof(Header);
                std::cout << "allocatedAddress" << allocatedAddress << '\n';
                return allocatedAddress;
            }
        }
    }
};

int main()
{
    MemAlloc alloc;
    alloc.allocate(20);
    return 0;
}
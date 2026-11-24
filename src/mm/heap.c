#include "../include/heap.h"
#include "../include/console.h"
#include "../include/mm.h"
#include "../include/types.h"

static block_header_t* heap_head = NULL;

static uintptr_t heap_bounds_start = 0;
static uintptr_t heap_bounds_end = 0;

/**
 * @brief Инициализирует кучу: создаёт один большой свободный блок.
 *
 * Берёт границы кучи из символов компоновщика @c _heap_start и
 * @c _heap_end, выравнивает их по 16 байт и превращает всю область
 * в единственный свободный блок.
 *
 * @warning Функцию нужно вызвать один раз до любого kmalloc/kfree.
 *          При слишком маленькой области (меньше одного заголовка)
 *          куча остаётся неинициализированной (heap_head == NULL),
 *          и kmalloc будет возвращать NULL.
 */
void heap_init(void) {
    heap_bounds_start = ALIGN16((uintptr_t)_heap_start);
    heap_bounds_end = (uintptr_t)_heap_end & ~((uintptr_t)15);

    if (heap_bounds_end <= heap_bounds_start + sizeof(block_header_t)) {
        heap_head = NULL;
        return;
    }

    heap_head = (block_header_t*)heap_bounds_start;
    heap_head->magic = HEAP_MAGIC_FREE;
    heap_head->size = (size_t)(heap_bounds_end - heap_bounds_start - sizeof(block_header_t));
    heap_head->is_free = 1;
    heap_head->next = NULL;
    // Отравляем весь payload, чтобы инвариант "свободное = 0xA5"
    // выполнялся с самого начала
    uint8_t* raw = (uint8_t*)heap_head + sizeof(block_header_t);
    for (size_t i = 0; i < heap_head->size; i++)
        raw[i] = HEAP_POISON_BYTE;
}

/**
 * @brief Выводит карту кучи: все блоки, их адреса, размеры и статус.
 *
 * Параллельно подсчитывает суммарный объём занятой и свободной памяти
 * (без учёта заголовков) и печатает итог. Удобно для отладки фрагментации
 * и проверки коалесценции после kfree.
 */
void heap_dump(void) {
    block_header_t* curr = heap_head;
    size_t total_free = 0;
    size_t total_used = 0;
    size_t block_idx = 0;

    kprintf("=== Карта кучи (Heap Map) ===\n");
    while (curr != NULL) {
        // Проверяем целостность заголовка при обходе
        const char* status = "CORRUPTED!";
        if (curr->magic == HEAP_MAGIC_FREE && curr->is_free) {
            status = "FREE";
            total_free += curr->size;
        } else if (curr->magic == HEAP_MAGIC_ALLOC && !curr->is_free) {
            status = "USED";
            total_used += curr->size;
        }

        kprintf("[%d] Block: 0x%x | Payload: 0x%x | Size: %d B | %s\n", (int)block_idx, (uintptr_t)curr,
                (uintptr_t)curr + sizeof(block_header_t), (int)curr->size, status);

        curr = curr->next;
        block_idx++;
    }
    kprintf("Всего блоков: %d | Занято: %d B | Свободно: %d B\n", (int)block_idx, (int)total_used, (int)total_free);
}

/**
 * @brief Нагрузочный тест аллокатора с демонстрацией коалесценции.
 *
 * Сценарий:
 * 1. Выделяются три блока (64, 128, 256 байт).
 * 2. Освобождается средний блок — в куче образуется «дыра».
 * 3. Освобождается первый блок — он должен слиться со средним (коалесценция
 *    с последующим соседом).
 * 4. Освобождается третий блок — вся куча должна свернуться в один
 *    свободный блок.
 */
void heap_test(void) {
    kprintf("=== 1. Тест выделения и коалесценции ===\n");
    uint8_t* p1 = (uint8_t*)kmalloc(64);
    uint8_t* p2 = (uint8_t*)kmalloc(128);
    uint8_t* p3 = (uint8_t*)kmalloc(256);

    if (p1) p1[0] = 0x11;
    if (p2) p2[0] = 0x22;
    if (p3) p3[0] = 0x33;

    heap_dump();

    kprintf("\n--- Освобождаем средний блок (p2) ---\n");
    kfree(p2);
    heap_dump();

    kprintf("\n--- Освобождаем p1 (слияние вперёд с p2) ---\n");
    kfree(p1);
    heap_dump();

    kprintf("\n--- Освобождаем p3 (слияние назад, куча должна стать единой) ---\n");
    kfree(p3);
    heap_dump();

    kprintf("\n=== 2. Тест безопасности (Security Checks) ===\n");
    kprintf("Попытка Double-Free на p3:\n");
    // cppcheck-suppress doubleFree
    kfree(p3);

    kprintf("Попытка освободить невыровненный указатель:\n");
    kfree((void*)0x80005003);

    kprintf("Попытка освободить указатель вне кучи (например, из стека/кода):\n");
    kfree((void*)0x80000010);
}

/**
 * @brief Выделяет блок памяти размером @p size байт (first-fit поиск).
 * @param size Запрашиваемый размер в байтах; 0 возвращает NULL.
 * @return Указатель на полезную нагрузку блока или NULL, если
 *         подходящий блок не найден (куча исчерпана или не инициализирована).
 *
 * Алгоритм:
 * 1. Размер выравнивается вверх до 16 байт.
 * 2. Список просматривается до первого свободного блока достаточного размера.
 * 3. Если в блоке остаётся запас минимум на заголовок + 16 байт полезной
 *    нагрузки, блок расщепляется: «хвост» становится новым свободным блоком.
 * 4. Иначе блок отдаётся целиком — пользователь получает больше,
 *    чем просил (внутренняя фрагментация).
 *
 * @note Сложность O(n) по числу блоков. Для небольшого ядра достаточно;
 *       при росте имеет смысл вести отдельный список свободных блоков.
 */
void* kmalloc(size_t size) {
    if (size == 0 || heap_head == NULL) return NULL;

    if (size > (size_t)-1 - sizeof(block_header_t) - 16) {
        kprintf("[SECURITY] kmalloc: переполнение размера, запрос отклонён\n");
        return NULL;
    }

    size = ALIGN16(size);
    block_header_t* curr = heap_head;

    while (curr != NULL) {
        // Проверка целостности кучи на лету (защита от повреждения метаданных)
        if (curr->magic != HEAP_MAGIC_FREE && curr->magic != HEAP_MAGIC_ALLOC) {
            kprintf("[PANIC] Heap corruption detected at 0x%x!\n", (uintptr_t)curr);
            for (;;) {
                __asm__ volatile("wfi");
            } // куча битая -- продолжать опасно -- бесконечный цикл с wfi
        }

        if (curr->is_free && curr->size >= size) {
            const uint8_t* probe = (const uint8_t*)curr + sizeof(block_header_t);
            for (int i = 0; i < HEAP_UAF_CHECK_BYTES; i++) {
                if (probe[i] != HEAP_POISON_BYTE) {
                    kprintf("[SECURITY] Use-after-free detected at 0x%x (payload перезаписан после kfree)\n",
                            (uintptr_t)curr);
                    break;
                }
            }

            if (curr->size >= size + sizeof(block_header_t) + 16) {
                block_header_t* new_block = (block_header_t*)((uint8_t*)curr + sizeof(block_header_t) + size);
                new_block->magic = HEAP_MAGIC_FREE;
                new_block->size = curr->size - size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = curr->next;

                curr->size = size;
                curr->next = new_block;
            }
            curr->is_free = 0;
            curr->magic = HEAP_MAGIC_ALLOC;
            return (void*)((uint8_t*)curr + sizeof(block_header_t));
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 * @brief Освобождает ранее выделенный блок и сливает его со свободными соседями.
 * @param ptr Указатель, возвращённый kmalloc (NULL игнорируется).
 *
 * Алгоритм:
 * 1. Проверяет валидность указателя (внутри границ кучи, выровнен по 16).
 * 2. Помечает блок свободным.
 * 3. Сливает блок с последующим соседом, если тот свободен.
 * 4. Находит предыдущий блок и сливает с ним, если он свободен.
 *
 * Защита от невалидных указателей: при выходе за границы кучи или
 * нарушении выравнивания печатает ошибку и не трогает память.
 *
 * @note Двойной kfree одного указателя этой проверкой не ловится —
 *       для полного контроля добавьте в заголовок магическое поле-страж
 *       (см. замечания ниже).
 */
void kfree(void* ptr) {
    if (ptr == NULL) return;

    uintptr_t addr = (uintptr_t)ptr;

    // 1. Проверка выравнивания указателя
    if ((addr & 15) != 0) {
        kprintf("[SECURITY] kfree: unaligned pointer 0x%x\n", addr);
        return;
    }

    // 2. Проверка попадания указателя в границы кучи
    if (addr < heap_bounds_start + sizeof(block_header_t) || addr >= heap_bounds_end) {
        kprintf("[SECURITY] kfree: pointer out of heap bounds 0x%x\n", addr);
        return;
    }

    block_header_t* block = (block_header_t*)(addr - sizeof(block_header_t));

    // 3. Классификация указателя: сначала точные состояния, потом мусор
    if (block->magic == HEAP_MAGIC_FREE && block->is_free == 1) {
        kprintf("[SECURITY] kfree: Double-Free detected at pointer: %p\n", ptr);
        return;
    }
    if (block->magic != HEAP_MAGIC_ALLOC || block->is_free != 0) {
        kprintf("[SECURITY] kfree: Invalid/corrupted header at pointer: %p\n", ptr);
        return;
    }

    // Помечаем блок как свободный
    block->is_free = 1;
    block->magic = HEAP_MAGIC_FREE;

    // Восстанавливаем инвариант "свободное = 0xA5" на проверяемой области,
    // чтобы UAF-детектор в kmalloc не срабатывал на переиспользовании
    uint8_t* payload = (uint8_t*)block + sizeof(block_header_t);
    for (size_t i = 0; i < HEAP_UAF_CHECK_BYTES; i++)
        payload[i] = HEAP_POISON_BYTE;

    // 4. Локальная коалесценция ВПЕРЁД (со следующим блоком)
    if (block->next != NULL && block->next->is_free && block->next->magic == HEAP_MAGIC_FREE) { //
        block_header_t* victim = block->next;
        size_t victim_size = victim->size;          // : сохраняем ПОЛЕ
        block_header_t* victim_next = victim->next; // : ДО затирания

        // вместо magic=0 пишем яд — заголовок становится
        // частью свободного payload, инвариант сохраняется
        uint8_t* raw = (uint8_t*)victim;
        for (size_t i = 0; i < sizeof(block_header_t); i++)
            raw[i] = HEAP_POISON_BYTE;

        block->size += sizeof(block_header_t) + victim_size;
        block->next = victim_next;
    }
    // 5. Локальная коалесценция НАЗАД (поиск предыдущего блока)
    block_header_t* blk_next = block->next; // сохраняем next до затирания
    block_header_t* prev = heap_head;
    while (prev != NULL && prev->next != block) {
        prev = prev->next;
    }
    if (prev != NULL && prev->is_free && prev->magic == HEAP_MAGIC_FREE) { //
        prev->size += sizeof(block_header_t) + block->size;

        // Заголовок тоже становится payload — ядовит
        uint8_t* raw = (uint8_t*)block;
        for (size_t i = 0; i < sizeof(block_header_t); i++)
            raw[i] = HEAP_POISON_BYTE;

        prev->next = blk_next;
    }
}

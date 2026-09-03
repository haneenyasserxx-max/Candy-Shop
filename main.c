#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CANDY_KINDS   6U
#define BASKET_MAX    8U
#define NAME_LEN     16U

typedef struct {
    char     name[NAME_LEN];
    uint16_t price;      /* piastres */
    uint16_t stock;      /* how many are left on the shelf */
    uint16_t sold;       /* how many we sold today         */
} Candy_t;

typedef struct {
    uint8_t  candyId;
    uint8_t  qty;
} Line_t;

static Candy_t shelf[CANDY_KINDS];
static Line_t  basket[BASKET_MAX];
static uint8_t basketLines;      
static uint32_t cashDrawer;

/* Safe input helper */
static int readInt(int *out) {
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) return 0;
    return sscanf(buf, "%d", out) == 1;
}

static void openShop(void) {
    basketLines = 0;
    cashDrawer = 0;

    strncpy(shelf[0].name, "Lollipop", NAME_LEN - 1);
    shelf[0].price = 250; shelf[0].stock = 50; shelf[0].sold = 0;

    strncpy(shelf[1].name, "Chocolate Bar", NAME_LEN - 1);
    shelf[1].price = 1000; shelf[1].stock = 20; shelf[1].sold = 0;

    strncpy(shelf[2].name, "Gummy Bears", NAME_LEN - 1);
    shelf[2].price = 500; shelf[2].stock = 30; shelf[2].sold = 0;

    strncpy(shelf[3].name, "Sour Belts", NAME_LEN - 1);
    shelf[3].price = 150; shelf[3].stock = 40; shelf[3].sold = 0;

    strncpy(shelf[4].name, "Jawbreaker", NAME_LEN - 1);
    shelf[4].price = 750; shelf[4].stock = 15; shelf[4].sold = 0;

    strncpy(shelf[5].name, "Mint Chews", NAME_LEN - 1);
    shelf[5].price = 50; shelf[5].stock = 100; shelf[5].sold = 0;
}

static void showShelf(void) {
    printf("\n--- Candy Shelf ---\n");
    for (uint8_t i = 0; i < CANDY_KINDS; i++) {
        printf("%u) %-15s | %4u pt | ", i, shelf[i].name, shelf[i].price);
        if (shelf[i].stock == 0) {
            printf("SOLD OUT\n");
        } else {
            printf("%u left\n", shelf[i].stock);
        }
    }
}

static void addToBasket(void) {
    int id = -1, qty = -1;
    
    printf("Which candy number? (0-%u): ", CANDY_KINDS - 1);
    if (!readInt(&id) || id < 0 || id >= (int)CANDY_KINDS) {
        printf("Invalid candy.\n");
        return;
    }

    printf("How many? ");
    if (!readInt(&qty) || qty <= 0) {
        printf("Invalid quantity.\n");
        return;
    }

    /* Check if we already have it in the basket to add up quantities */
    int foundIndex = -1;
    for (uint8_t i = 0; i < basketLines; i++) {
        if (basket[i].candyId == (uint8_t)id) {
            foundIndex = i;
            break;
        }
    }

    uint16_t currentInBasket = (foundIndex != -1) ? basket[foundIndex].qty : 0;
    
    if ((currentInBasket + qty) > shelf[id].stock) {
        printf("Not enough stock! Only %u available on the shelf.\n", shelf[id].stock);
        return;
    }

    if (foundIndex != -1) {
        basket[foundIndex].qty += (uint8_t)qty;
        printf("Added %d more %s to your basket.\n", qty, shelf[id].name);
    } else {
        if (basketLines >= BASKET_MAX) {
            printf("Basket is full! Max %u items allowed.\n", BASKET_MAX);
            return;
        }
        basket[basketLines].candyId = (uint8_t)id;
        basket[basketLines].qty = (uint8_t)qty;
        basketLines++;
        printf("Added %d %s to your basket.\n", qty, shelf[id].name);
    }
}

static void removeFromBasket(void) {
    if (basketLines == 0) {
        printf("Basket is already empty.\n");
        return;
    }

    int line = -1;
    printf("Remove which basket line? (0-%u): ", basketLines - 1);
    if (!readInt(&line) || line < 0 || line >= basketLines) {
        printf("Invalid line number.\n");
        return;
    }

    /* Slide everything up to fill the gap */
    for (uint8_t i = (uint8_t)line; i < basketLines - 1; i++) {
        basket[i] = basket[i + 1];
    }
    basketLines--;
    printf("Line removed.\n");
}

static uint32_t basketTotal(void) {
    uint32_t total = 0;
    for (uint8_t i = 0; i < basketLines; i++) {
        uint8_t id = basket[i].candyId;
        total += (uint32_t)basket[i].qty * shelf[id].price;
    }
    return total;
}

static void showBasket(void) {
    printf("\n--- Current Basket ---\n");
    if (basketLines == 0) {
        printf("Basket is empty.\n");
        return;
    }

    for (uint8_t i = 0; i < basketLines; i++) {
        uint8_t id = basket[i].candyId;
        uint32_t lineCost = (uint32_t)basket[i].qty * shelf[id].price;
        printf("Line %u: %-15s x%u @ %u pt = %lu pt\n", 
               i, shelf[id].name, basket[i].qty, shelf[id].price, (unsigned long)lineCost);
    }
    printf("----------------------\n");
    printf("TOTAL: %lu piastres\n", (unsigned long)basketTotal());
}

static void giveChange(uint32_t change) {
    if (change == 0) {
        printf("No change, thank you!\n");
        return;
    }

    printf("Change due: %lu pt\nHanding back:\n", (unsigned long)change);
    
    uint16_t coins[] = {500, 200, 100, 50, 25};
    for (int i = 0; i < 5; i++) {
        uint32_t count = change / coins[i];
        if (count > 0) {
            printf("  %lu x %u pt coin(s)\n", (unsigned long)count, coins[i]);
            change = change % coins[i];
        }
    }

    /* What happens if we owe 137 pt and the smallest coin is 25? */
    if (change > 0) {
        printf("  ...and shop keeps %lu pt (We don't have coins smaller than 25 pt!)\n", (unsigned long)change);
    }
}

static void checkout(void) {
    if (basketLines == 0) {
        printf("Basket is empty! Nothing to checkout.\n");
        return;
    }

    uint32_t total = basketTotal();
    printf("\nTotal is %lu piastres.\n", (unsigned long)total);
    
    int cash = 0;
    printf("Cash handed over: ");
    if (!readInt(&cash) || cash < 0) {
        printf("Invalid cash amount. Checkout aborted.\n");
        return;
    }

    if ((uint32_t)cash < total) {
        printf("Not enough money! You are short %lu pt. Checkout aborted.\n", (unsigned long)(total - cash));
        return;
    }

    /* Process the sale */
    for (uint8_t i = 0; i < basketLines; i++) {
        uint8_t id = basket[i].candyId;
        shelf[id].stock -= basket[i].qty;
        shelf[id].sold += basket[i].qty;
    }

    cashDrawer += total;
    uint32_t change = (uint32_t)cash - total;
    
    printf("\nPayment successful!\n");
    giveChange(change);
    
    /* Empty the basket */
    basketLines = 0;
}

static uint8_t bestSeller(void) {
    uint8_t bestId = 0;
    for (uint8_t i = 1; i < CANDY_KINDS; i++) {
        if (shelf[i].sold > shelf[bestId].sold) {
            bestId = i;
        }
    }
    return bestId;
}

static void dayReport(void) {
    uint32_t totalSoldItems = 0;
    for (uint8_t i = 0; i < CANDY_KINDS; i++) {
        totalSoldItems += shelf[i].sold;
    }

    uint8_t best = bestSeller();

    printf("\n=== END OF DAY REPORT ===\n");
    printf("Cash in Drawer   : %lu piastres\n", (unsigned long)cashDrawer);
    printf("Total Candies Sold: %lu\n", (unsigned long)totalSoldItems);
    
    if (totalSoldItems > 0) {
        printf("Best Seller      : %s (%u sold)\n", shelf[best].name, shelf[best].sold);
    } else {
        printf("Best Seller      : Nothing sold today!\n");
    }

    printf("Sold Out Items   :\n");
    uint8_t soldOutCount = 0;
    for (uint8_t i = 0; i < CANDY_KINDS; i++) {
        if (shelf[i].stock == 0) {
            printf(" - %s\n", shelf[i].name);
            soldOutCount++;
        }
    }
    if (soldOutCount == 0) printf(" - (None)\n");
    printf("=========================\n");
}

int main(void) {
    openShop();
    int choice = -1;

    do {
        printf("\n============================\n");
        printf("       CANDY COUNTER        \n");
        printf("============================\n");
        printf(" 1) Show Shelf\n");
        printf(" 2) Add to Basket\n");
        printf(" 3) Remove from Basket\n");
        printf(" 4) Show Basket\n");
        printf(" 5) Checkout\n");
        printf(" 6) End of Day Report\n");
        printf(" 0) Close Shop (Exit)\n");
        printf("Select > ");

        if (!readInt(&choice)) {
            printf("Invalid input.\n");
            continue;
        }

        switch (choice) {
            case 1: showShelf(); break;
            case 2: addToBasket(); break;
            case 3: removeFromBasket(); break;
            case 4: showBasket(); break;
            case 5: checkout(); break;
            case 6: dayReport(); break;
            case 0: printf("Closing up shop...\n"); break;
            default: printf("Unknown option.\n"); break;
        }
    } while (choice != 0);

    return 0;
}
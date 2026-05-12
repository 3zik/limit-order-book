# Order book notes

Conceptual background for how exchanges organize orders, plus how **this repository** maps to those ideas.

## Price–time priority (L3 / MBO)

- Buy and sell interest for an instrument is organized by **price**, then **time** at each price.
- Example: many buyers at different prices. A seller matches against the **best bid** first; at the same price, earlier orders are ahead of later ones (**FIFO**).
- **This project:** FIFO at each price is implemented with `std::list` per price level; `std::map` orders prices so the best bid/ask are at `begin()`.

## Market depth levels (terminology)

### L1

- **BBO** — best bid and best offer (highest bid, lowest ask).

### L2 (market by price, MBP)

- Per price: typically **total displayed quantity** (and sometimes order count).
- **This project:** `GetOrderInfos()` returns aggregated **remaining quantity** per price on each side (`LevelInfo` / `OrderbookLevelInfos`). It does not expose per-order IDs (that would be closer to L3).

### L3 (market by order, MBO)

- Individual order queue and priority rules.
- **Queue styles (general industry):**
  - **Time–price priority (FIFO)** at a price — what this code implements.
  - **Pro-rata** — size-weighted allocation at a price; **not** implemented here.

## Order and TIF types

### Limit

- Execute at this price **or better**; unfilled quantity can rest in the book.
- **This project:** standard GTC path after optional FOK/FAK checks.

### Market (typical intent vs this code)

- Colloquial: “take liquidity now at the best available prices,” often walking the book.
- **This project:** if the opposite side exists, the order becomes **GTC at the worst price on that opposite book** (`rbegin()`), then participates in normal matching. It is **not** a multi-level aggressive sweep; treat it as a simplified stand-in.

### Time in force (TIF)

- **Good-Till-Cancel (GTC)** — rests until filled or canceled.
- **Day / GFD** — cancel if not done by end of session or day.
  - **This project:** GFD orders are canceled by a **background thread** that wakes on a timer derived from local wall clock (see `PruneGoodForDayOrders`); not exchange-calendar aware.
- **Fill-and-Kill (FAK / IOC)** — fill what you can immediately; **cancel any remainder** (here: remainder at the inside price after matching is canceled).
- **Fill-or-Kill (FOK)** — fill **entire** size immediately or **do not place** the order.
- **AON (all-or-none, resting)** — like FOK in spirit but may **wait** in the book; **not** implemented (FOK here is reject-before-rest only).

### Order types listed but not implemented here

- **Stop** and **Stop-Limit** — trigger logic is not in this codebase; notes above are for general literacy only.

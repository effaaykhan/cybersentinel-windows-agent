# Windows Agent Classification Algorithm

## Classification Rules

The Windows agent uses regex-based pattern matching to classify sensitive data. The algorithm works as follows:

### Severity Levels

1. **CRITICAL** - Highest priority, immediate attention required
2. **HIGH** - Important, should be reviewed
3. **MEDIUM** - Moderate risk
4. **LOW** - Default, no sensitive data detected

### Detection Patterns

#### 1. Credit Card (PAN) - **CRITICAL**
- **Pattern:** `\b\d{4}[\s-]?\d{4}[\s-]?\d{4}[\s-]?\d{4}\b`
- **Example:** `1234 5678 9012 3456` or `1234-5678-9012-3456`
- **Label:** `PAN`
- **Severity:** `critical`

#### 2. Social Security Number (SSN) - **CRITICAL**
- **Pattern:** `\b\d{3}-\d{2}-\d{4}\b`
- **Example:** `123-45-6789`
- **Label:** `SSN`
- **Severity:** `critical`

#### 3. Email Address - **MEDIUM**
- **Pattern:** `\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b`
- **Example:** `test@example.com`
- **Label:** `EMAIL`
- **Severity:** `medium` (only if no higher severity found)

#### 4. API Keys / Secrets - **HIGH**
- **Pattern:** `api[_-]?key|secret[_-]?key|access[_-]?token` (case-insensitive)
- **Example:** `api_key=abc123`, `secret-key: xyz`, `ACCESS_TOKEN=token`
- **Label:** `API_KEY`
- **Severity:** `high`

### Classification Score

- **If labels found:** `score = 0.9`
- **If no labels:** `score = 0.1`

### Algorithm Flow

1. Start with `severity = "low"`
2. Check patterns in order (highest priority first):
   - Credit card → set to `critical`
   - SSN → set to `critical`
   - Email → set to `medium` (only if still `low`)
   - API key → set to `high`
3. Return classification with labels, severity, and score

### Example Classifications

- **File with SSN:** `123-45-6789` → `critical`, labels: `["SSN"]`, score: `0.9`
- **File with email:** `test@example.com` → `medium`, labels: `["EMAIL"]`, score: `0.9`
- **File with both:** `test@example.com and 123-45-6789` → `critical`, labels: `["SSN", "EMAIL"]`, score: `0.9`
- **Normal file:** No patterns → `low`, labels: `[]`, score: `0.1`


# Itch Plugin - Roadmap

## Current Status (September 2025)
✅ **Phase 1 Complete**: Core Purchase Verification
- Download key validation and verification  
- Clean API interface with comprehensive error handling
- Enhanced settings panel and complete test suite
- Multi-format input support and async verification

## Planned Development

## Planned Development

### Phase 2: Enhanced User Experience (Q4 2025)
- **Pre-built Verification Scene** - Drop-in verification UI
- **Editor Test Connection Tool** - Built-in editor button for API key/Game ID verification
- **Theme Support** - Customizable styling
- **Improved Caching** - Persistent cache with offline mode
- **OAuth Authentication** - User login flow for games without download keys
- **Developer Tools** - Visual editor integration and setup wizard

### Phase 3: Complete itch.io API Integration (Q1-Q2 2026)
- **Full Server-side API Support**:
  - User profile access (`/me` endpoint)
  - Game library management (`/my-games` endpoint) 
  - Purchase history lookup (`/game/ID/purchases` endpoint)
  - Download key management with email/user_id lookup
- **OAuth 2.0 Implementation**:
  - Complete OAuth flow for user authentication
  - Scope-based permissions (profile:me, future scopes)
  - Secure token management and refresh
- **Security Enhancements** - Enhanced encryption and fraud detection
- **Platform Expansion** - Steam integration and mobile optimization
- **Analytics & Insights** - Usage tracking and error reporting

### Phase 4: Enterprise & Advanced Features (Q3-Q4 2026)
- **Multi-Store Support** - Verify purchases from multiple platforms
- **Advanced Game Management**:
  - Bulk download key generation and distribution
  - Purchase analytics and revenue tracking
  - Customer relationship management
- **LoQ (Line of Questioning) Features**:
  - Interactive purchase verification interviews
  - Adaptive verification flows based on user behavior
  - Multi-factor authentication for high-value purchases
  - Smart fraud detection with ML-powered questioning
- **Admin Dashboard** - Web-based management interface
- **Enterprise Integration** - API extensions and custom workflows
- **Scalability** - CDN integration and load balancing

## Technical Evolution

### Current Architecture
```
Itch (API) → Itch (Autoload) → VerificationClient → itch.io API
```

### Future Architecture (Phase 4)
```
Itch Enterprise Platform
├── Multi-Platform Verification Hub
├── Advanced Analytics & BI
├── Enterprise Security Suite
└── Developer Experience Platform
```

## API Evolution
- **v1.0** (Current): Simple verification interface
- **v2.0** (Phase 2): Enhanced options, UI components, and OAuth support
- **v3.0** (Phase 3): Complete itch.io API integration and multi-platform support
- **v4.0** (Phase 4): Enterprise features, LoQ systems, and advanced analytics

## Missing itch.io API Features (To Be Implemented)

### Currently Missing from Plugin:
1. **User Profile Management** (`/me` endpoint)
   - Access to user's public profile data
   - Developer status verification
   - User preferences and settings

2. **Game Library Access** (`/my-games` endpoint)
   - List all user's uploaded games
   - Game statistics (downloads, purchases, views)
   - Revenue and earnings data
   - Game metadata management

3. **Advanced Purchase Verification** (`/game/ID/purchases` endpoint)
   - Email-based purchase lookup
   - User ID-based verification
   - Purchase history and transaction details
   - Source tracking (Amazon, direct, etc.)

4. **OAuth Authentication Flow**
   - Complete OAuth 2.0 implementation
   - User consent and permission management
   - Secure token handling and refresh
   - Scope-based access control

5. **Credential Management** (`/credentials/info` endpoint)
   - API key validation and scope checking
   - Token expiration monitoring
   - Permission verification

6. **Enhanced Download Key Features**
   - Bulk key generation and management
   - Email and user ID-based key lookup
   - Key usage analytics and tracking
   - Revocation and expiration handling

## LoQ (Line of Questioning) Feature Specifications

### Adaptive Verification System
- **Smart Interview Flows**: Dynamic questioning based on purchase patterns
- **Behavioral Analysis**: Detect suspicious verification attempts
- **Multi-Factor Challenges**: Combine email, purchase date, and game-specific questions
- **Risk Assessment**: Score verification attempts and apply appropriate challenges

### Implementation Examples:
```gdscript
# Adaptive verification flow
Itch.start_loq_verification(download_key, {
    "risk_level": "medium",
    "challenge_types": ["email", "purchase_date", "game_knowledge"],
    "max_attempts": 3
})

# Smart questioning system
Itch.configure_loq_questions({
    "game_specific": ["What level did you reach?", "Favorite character?"],
    "purchase_history": ["When did you buy this?", "What payment method?"],
    "behavioral": ["How did you hear about this game?"]
})
```

## Success Metrics
- **Phase 2**: 50+ games using pre-built UI, <10min setup time, OAuth integration
- **Phase 3**: Complete itch.io API coverage, 25+ multi-platform games, 500+ community members
- **Phase 4**: 10+ enterprise customers, LoQ system deployment, industry standard recognition

## Detailed Feature Specifications

### Phase 2: Editor Test Connection Tool
**Implementation**: Add verification button directly in Godot editor's Project Settings
**Features**:
- Real-time API key validation using `/credentials/info` endpoint
- Game ID verification against user's game library
- Connection status indicators with detailed error messages
- One-click setup assistance for common configuration issues

**Technical Approach**:
```gdscript
# Editor plugin integration
func test_connection_in_editor():
   var config = ItchConfig.new()
    var result = await GodotItch.test_credentials(config.get_api_key(), config.get_game_id())
    
    if result.success:
        show_success_notification("✅ Connection successful!")
    else:
        show_error_dialog("❌ " + result.error_message)
```

### Phase 3: OAuth Implementation
**Objective**: Enable user authentication without requiring download keys
**Flow**:
1. Game requests OAuth permissions with specific scopes
2. User redirected to itch.io authorization page
3. User grants permissions and receives access token
4. Game uses token for API requests on user's behalf

**Security Features**:
- HTTPS-only callback URLs
- State parameter for CSRF protection
- Automatic token refresh and expiration handling
- Secure local token storage

### Phase 4: LoQ (Line of Questioning) System
**Advanced Verification Scenarios**:
- **Suspicious Activity Detection**: Multiple failed attempts, unusual patterns
- **High-Value Purchase Protection**: Expensive games, bundle purchases
- **Account Recovery**: Lost download keys, email changes
- **Fraud Prevention**: Shared keys, reseller detection

**Adaptive Question Types**:
- **Temporal**: "When did you purchase this game?"
- **Contextual**: "What was the last level you completed?"
- **Personal**: "What's your favorite feature in this game?"
- **Technical**: "Which platform did you download from?"

## Long-term Vision (2027+)
- Universal verification for all digital distribution platforms
- Zero-configuration setup with AI-powered optimization
- Blockchain integration and quantum-ready security

---
**Last Updated**: September 8, 2025  
This roadmap evolves based on community feedback and market opportunities.
